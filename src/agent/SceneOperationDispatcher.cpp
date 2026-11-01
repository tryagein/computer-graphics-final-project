#include "agent/SceneOperationDispatcher.hpp"
#include "agent/SceneStateQuery.hpp"
#include "assets/AssetService.hpp"
#include "assets/PrefabCatalog.hpp"
#include "persistence/SceneFileService.hpp"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <set>
#include <stdexcept>

namespace scene_editor {
namespace {
using nlohmann::json;
namespace fs = std::filesystem;

void allowOnly(const json& arguments, std::initializer_list<const char*> keys) {
    std::set<std::string> allowed(keys.begin(), keys.end());
    for (auto it = arguments.begin(); it != arguments.end(); ++it)
        if (!allowed.count(it.key())) throw std::runtime_error("Unsupported argument: " + it.key());
}

std::string requiredString(const json& args, const char* key) {
    const auto value = args.at(key).get<std::string>();
    if (value.empty() || value.size() > 256) throw std::runtime_error(std::string("Invalid ") + key);
    return value;
}

void optionalVector(const json& args, const char* key, glm::vec3& value) {
    if (!args.contains(key)) return;
    const auto& raw = args.at(key);
    if (!raw.is_array() || raw.size() != 3) throw std::runtime_error(std::string(key) + " needs 3 numbers.");
    value = {raw.at(0).get<float>(), raw.at(1).get<float>(), raw.at(2).get<float>()};
    for (int i = 0; i < 3; ++i)
        if (!std::isfinite(value[i])) throw std::runtime_error(std::string(key) + " must be finite.");
}

void optionalColor(const json& args, const char* key, glm::vec3& value) {
    optionalVector(args, key, value);
    if (args.contains(key)) for (int i = 0; i < 3; ++i)
        if (value[i] < 0 || value[i] > 1) throw std::runtime_error(std::string(key) + " must be in [0,1].");
}

std::string newId(const Scene& scene, const std::string& prefix) {
    for (unsigned i = 1; i < 100000; ++i) {
        const auto id = prefix + std::to_string(i);
        const bool usedByObject = std::any_of(scene.objects.begin(), scene.objects.end(),
            [&](const SceneObject& o) { return o.id == id; });
        const bool usedByLight = std::any_of(scene.lights.begin(), scene.lights.end(),
            [&](const Light& l) { return l.id == id; });
        if (!usedByObject && !usedByLight) return id;
    }
    throw std::runtime_error("No free scene ID remains.");
}

bool idUsed(const Scene& scene, const std::string& id) {
    return std::any_of(scene.objects.begin(), scene.objects.end(),
        [&](const SceneObject& o) { return o.id == id; }) ||
        std::any_of(scene.lights.begin(), scene.lights.end(),
        [&](const Light& l) { return l.id == id; });
}

fs::path agentFile(const std::string& filename) {
    const fs::path name = fs::u8path(filename);
    if (name.is_absolute() || name.has_parent_path() || filename.empty() ||
        filename.find_first_of("<>:\"/\\|?*") != std::string::npos)
        throw std::runtime_error("Use a scene filename without a directory.");
    const auto finalName = name.extension() == ".json" ? name : fs::u8path(filename + ".json");
    return fs::current_path() / "scenes" / finalName;
}

Light* findLight(Scene& scene, const std::string& id) {
    auto it = std::find_if(scene.lights.begin(), scene.lights.end(),
        [&](const Light& light) { return light.id == id; });
    return it == scene.lights.end() ? nullptr : &*it;
}
}

OperationResult dispatchSceneOperation(const SceneOperation& operation, Scene& scene, Camera& camera) {
    const auto& args = operation.arguments;
    try {
        if (operation.tool == "add_prefab") {
            allowOnly(args, {"asset_id", "id", "position"});
            glm::vec3 position{0};
            optionalVector(args, "position", position);
            const auto result = addPrefab(scene, requiredString(args, "asset_id"), position,
                args.contains("id") ? requiredString(args, "id") : std::string{});
            return {result.ok, false, result.ok, result.message,
                {{"object_id", result.objectId}, {"light_id", result.lightId}}};
        }
        if (operation.tool == "add_object") {
            allowOnly(args, {"type", "id", "name", "position", "rotation", "scale", "path"});
            const auto type = requiredString(args, "type");
            if (type == "imported_model") {
                const auto path = requiredString(args, "path");
                SceneObject candidate;
                candidate.kind = ObjectKind::ImportedModel;
                candidate.geometryPath = path;
                optionalVector(args, "position", candidate.transform.position);
                optionalVector(args, "rotation", candidate.transform.rotationDegrees);
                optionalVector(args, "scale", candidate.transform.scale);
                if (!candidate.transform.valid())
                    return {false, false, false, "Invalid imported model transform."};
                const auto requestedId = args.contains("id") ? requiredString(args, "id") : std::string{};
                if (!requestedId.empty() && idUsed(scene, requestedId))
                    return {false, false, false, "Object ID already exists: " + requestedId};
                const auto requestedName = args.contains("name") ? requiredString(args, "name") : std::string{};
                std::string error;
                if (!addImportedObj(scene, path, error)) return {false, false, false, error};
                auto* imported = scene.find(scene.selectedId);
                imported->transform = candidate.transform;
                if (!requestedName.empty()) imported->name = requestedName;
                if (!requestedId.empty()) { imported->id = requestedId; scene.selectedId = requestedId; }
                return {true, false, true, "Imported model added.", {{"object_id", scene.selectedId}}};
            }
            SceneObject object;
            if (type == "cube") object.kind = ObjectKind::Cube;
            else if (type == "sphere") object.kind = ObjectKind::Sphere;
            else if (type == "plane") object.kind = ObjectKind::Plane;
            else return {false, false, false, "Unsupported object type: " + type};
            object.id = args.contains("id") ? requiredString(args, "id") : newId(scene, "agent-object-");
            if (idUsed(scene, object.id)) return {false, false, false, "Object ID already exists: " + object.id};
            object.name = args.contains("name") ? requiredString(args, "name") : object.id;
            optionalVector(args, "position", object.transform.position);
            optionalVector(args, "rotation", object.transform.rotationDegrees);
            optionalVector(args, "scale", object.transform.scale);
            if (!scene.add(object)) return {false, false, false, "Invalid object transform or material."};
            scene.selectedId = object.id;
            return {true, false, false, "Object added.", {{"object_id", object.id}}};
        }
        if (operation.tool == "update_object" || operation.tool == "delete_object" ||
            operation.tool == "set_material") {
            const auto target = requiredString(args, "object_id");
            const auto resolved = resolveSceneObject(scene, target);
            if (!resolved.object) return {false, true, false, resolved.error};
            if (operation.tool == "delete_object") {
                allowOnly(args, {"object_id"});
                scene.erase(resolved.object->id);
                return {true, false, false, "Object deleted."};
            }
            SceneObject candidate = *resolved.object;
            if (operation.tool == "update_object") {
                allowOnly(args, {"object_id", "name", "position", "rotation", "scale", "visible"});
                if (args.contains("name")) candidate.name = requiredString(args, "name");
                optionalVector(args, "position", candidate.transform.position);
                optionalVector(args, "rotation", candidate.transform.rotationDegrees);
                optionalVector(args, "scale", candidate.transform.scale);
                if (args.contains("visible")) candidate.visible = args.at("visible").get<bool>();
            } else {
                allowOnly(args, {"object_id", "color", "ambient", "diffuse", "specular",
                                 "emission", "emission_strength", "shininess", "roughness", "texture_path"});
                auto& m = candidate.material;
                optionalColor(args, "color", m.color);
                optionalColor(args, "ambient", m.ambient);
                optionalColor(args, "diffuse", m.diffuse);
                optionalColor(args, "specular", m.specular);
                optionalColor(args, "emission", m.emission);
                if (args.contains("emission_strength"))
                    m.emissionStrength = args.at("emission_strength").get<float>();
                if (args.contains("shininess")) m.shininess = args.at("shininess").get<float>();
                if (args.contains("roughness")) {
                    const float roughness = args.at("roughness").get<float>();
                    if (!std::isfinite(roughness) || roughness < 0 || roughness > 1)
                        throw std::runtime_error("roughness must be in [0,1].");
                    m.shininess = 1.0F + 255.0F * std::pow(1.0F - roughness, 4.0F);
                }
                if (args.contains("texture_path")) {
                    m.texturePath = args.at("texture_path").get<std::string>();
                    if (!m.texturePath.empty() && !fs::is_regular_file(fs::u8path(m.texturePath)))
                        throw std::runtime_error("Texture file does not exist.");
                }
                candidate.materialOverride = true;
            }
            if (!candidate.valid()) return {false, false, false, "Invalid object or material values."};
            *resolved.object = std::move(candidate);
            return {true, false, false, operation.tool == "set_material" ? "Material updated." : "Object updated."};
        }
        if (operation.tool == "add_light") {
            allowOnly(args, {"id", "type", "position", "direction", "color", "intensity", "enabled"});
            const auto type = requiredString(args, "type");
            Light light;
            if (type == "point") light.type = LightType::Point;
            else if (type == "directional") light.type = LightType::Directional;
            else return {false, false, false, "Unsupported light type: " + type};
            light.id = args.contains("id") ? requiredString(args, "id") : newId(scene, "agent-light-");
            if (idUsed(scene, light.id)) return {false, false, false, "Light ID already exists."};
            optionalVector(args, "position", light.position);
            optionalVector(args, "direction", light.direction);
            optionalColor(args, "color", light.color);
            if (args.contains("intensity")) light.intensity = args.at("intensity").get<float>();
            if (args.contains("enabled")) light.enabled = args.at("enabled").get<bool>();
            const int count = int(std::count_if(scene.lights.begin(), scene.lights.end(),
                [&](const Light& item) { return item.type == light.type; }));
            if (count >= (light.type == LightType::Point ? 4 : 2))
                return {false, false, false, "Shader light limit reached."};
            if (!light.valid()) return {false, false, false, "Invalid light values."};
            scene.lights.push_back(light);
            return {true, false, false, "Light added.", {{"light_id", light.id}}};
        }
        if (operation.tool == "update_light" || operation.tool == "delete_light") {
            const auto id = requiredString(args, "light_id");
            auto* light = findLight(scene, id);
            if (!light) return {false, true, false, "Light not found: " + id};
            if (operation.tool == "delete_light") {
                allowOnly(args, {"light_id"});
                scene.lights.erase(std::remove_if(scene.lights.begin(), scene.lights.end(),
                    [&](const Light& item) { return item.id == id; }), scene.lights.end());
                return {true, false, false, "Light deleted."};
            }
            allowOnly(args, {"light_id", "position", "direction", "color", "intensity", "enabled"});
            Light candidate = *light;
            optionalVector(args, "position", candidate.position);
            optionalVector(args, "direction", candidate.direction);
            optionalColor(args, "color", candidate.color);
            if (args.contains("intensity")) candidate.intensity = args.at("intensity").get<float>();
            if (args.contains("enabled")) candidate.enabled = args.at("enabled").get<bool>();
            if (!candidate.valid()) return {false, false, false, "Invalid light values."};
            *light = candidate;
            return {true, false, false, "Light updated."};
        }
        if (operation.tool == "move_camera") {
            allowOnly(args, {"position", "target", "yaw", "pitch", "field_of_view", "near_plane", "far_plane"});
            Camera candidate = camera;
            glm::vec3 position = camera.position();
            optionalVector(args, "position", position);
            float yaw = args.value("yaw", camera.yaw());
            float pitch = args.value("pitch", camera.pitch());
            if (args.contains("target")) {
                glm::vec3 target{};
                optionalVector(args, "target", target);
                const auto direction = target - position;
                if (glm::length(direction) < 1e-6F) return {false, false, false, "Camera target equals position."};
                yaw = glm::degrees(std::atan2(direction.z, direction.x));
                pitch = glm::degrees(std::asin(glm::clamp(direction.y / glm::length(direction), -1.0F, 1.0F)));
            }
            if (!candidate.setPose(position, yaw, pitch) ||
                !candidate.setProjection(args.value("field_of_view", camera.fovDegrees()),
                                         args.value("near_plane", camera.nearPlane()),
                                         args.value("far_plane", camera.farPlane())))
                return {false, false, false, "Invalid camera pose or projection."};
            camera = candidate;
            return {true, false, false, "Camera updated."};
        }
        if (operation.tool == "get_scene_state") {
            allowOnly(args, {});
            return {true, false, false, "Scene state returned.", compactSceneState(scene, camera)};
        }
        if (operation.tool == "render_scene") {
            allowOnly(args, {});
            return {true, false, false, "Scene will render on the next frame.",
                    {{"object_count", scene.objects.size()}}};
        }
        if (operation.tool == "save_scene" || operation.tool == "load_scene") {
            allowOnly(args, {"filename"});
            const auto file = agentFile(requiredString(args, "filename"));
            const Result result = operation.tool == "save_scene" ? saveSceneFile(file, scene, camera)
                                                                  : loadSceneFile(file, scene, camera);
            return {result.ok, false, result.ok && operation.tool == "load_scene", result.message};
        }
        return {false, false, false, "Unsupported operation."};
    } catch (const std::exception& error) {
        return {false, false, false, std::string("Invalid operation arguments: ") + error.what()};
    }
}
}
