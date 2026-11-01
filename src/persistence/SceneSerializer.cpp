#include "persistence/SceneSerializer.hpp"
#include <stdexcept>

namespace scene_editor {
namespace {
using nlohmann::json;

json vectorJson(const glm::vec3& value) {
    return json::array({value.x, value.y, value.z});
}

glm::vec3 readVector(const json& value) {
    if (!value.is_array() || value.size() != 3) throw std::runtime_error("expected a three-component vector");
    return {value.at(0).get<float>(), value.at(1).get<float>(), value.at(2).get<float>()};
}

const char* kindName(ObjectKind kind) {
    switch (kind) {
    case ObjectKind::Cube: return "cube";
    case ObjectKind::Sphere: return "sphere";
    case ObjectKind::Plane: return "plane";
    case ObjectKind::ImportedModel: return "imported_model";
    }
    throw std::runtime_error("unsupported object kind");
}

ObjectKind readKind(const std::string& value) {
    if (value == "cube") return ObjectKind::Cube;
    if (value == "sphere") return ObjectKind::Sphere;
    if (value == "plane") return ObjectKind::Plane;
    if (value == "imported_model") return ObjectKind::ImportedModel;
    throw std::runtime_error("unsupported object kind: " + value);
}

const char* lightName(LightType type) {
    return type == LightType::Point ? "point" : "directional";
}

LightType readLightType(const std::string& value) {
    if (value == "point") return LightType::Point;
    if (value == "directional") return LightType::Directional;
    throw std::runtime_error("unsupported light type: " + value);
}

const char* resourceName(SceneResource::Kind kind) {
    switch (kind) {
    case SceneResource::Kind::Model: return "model";
    case SceneResource::Kind::MaterialLibrary: return "material_library";
    case SceneResource::Kind::Texture: return "texture";
    }
    throw std::runtime_error("unsupported resource kind");
}

SceneResource::Kind readResourceKind(const std::string& value) {
    if (value == "model") return SceneResource::Kind::Model;
    if (value == "material_library") return SceneResource::Kind::MaterialLibrary;
    if (value == "texture") return SceneResource::Kind::Texture;
    throw std::runtime_error("unsupported resource kind: " + value);
}
}

nlohmann::json serializeSceneDocument(const SceneDocument& document) {
    json output = {
        {"schema_version", document.schemaVersion},
        {"objects", json::array()},
        {"lights", json::array()},
        {"resources", json::array()},
        {"selected_id", document.scene.selectedId},
        {"camera", {
            {"position", vectorJson(document.camera.position())},
            {"yaw", document.camera.yaw()},
            {"pitch", document.camera.pitch()},
            {"field_of_view", document.camera.fovDegrees()},
            {"near_plane", document.camera.nearPlane()},
            {"far_plane", document.camera.farPlane()}
        }}
    };
    for (const auto& object : document.scene.objects) {
        output["objects"].push_back({
            {"id", object.id}, {"display_name", object.name},
            {"kind", kindName(object.kind)}, {"visible", object.visible},
            {"geometry_ref", object.geometryPath},
            {"material_override", object.materialOverride},
            {"transform", {
                {"position", vectorJson(object.transform.position)},
                {"rotation_degrees", vectorJson(object.transform.rotationDegrees)},
                {"scale", vectorJson(object.transform.scale)}
            }},
            {"material", {
                {"base_color", vectorJson(object.material.color)},
                {"ambient", vectorJson(object.material.ambient)},
                {"diffuse", vectorJson(object.material.diffuse)},
                {"specular", vectorJson(object.material.specular)},
                {"emission", vectorJson(object.material.emission)},
                {"emission_strength", object.material.emissionStrength},
                {"shininess", object.material.shininess},
                {"texture_ref", object.material.texturePath}
            }}
        });
    }
    for (const auto& light : document.scene.lights) {
        output["lights"].push_back({
            {"id", light.id}, {"type", lightName(light.type)},
            {"position", vectorJson(light.position)},
            {"direction", vectorJson(light.direction)},
            {"color", vectorJson(light.color)},
            {"intensity", light.intensity}, {"enabled", light.enabled}
        });
    }
    for (const auto& resource : document.resources)
        output["resources"].push_back({
            {"relative_path", resource.relativePath},
            {"kind", resourceName(resource.kind)}
        });
    return output;
}

Result deserializeSceneDocument(const nlohmann::json& input, SceneDocument& document) {
    try {
        SceneDocument parsed;
        parsed.schemaVersion = input.at("schema_version").get<int>();
        const auto& camera = input.at("camera");
        if (!parsed.camera.setPose(readVector(camera.at("position")),
                                   camera.at("yaw").get<float>(),
                                   camera.at("pitch").get<float>()) ||
            !parsed.camera.setProjection(camera.at("field_of_view").get<float>(),
                                         camera.at("near_plane").get<float>(),
                                         camera.at("far_plane").get<float>()))
            return Result::failure("Invalid camera pose or projection.");
        for (const auto& entry : input.at("objects")) {
            SceneObject object;
            object.id = entry.at("id").get<std::string>();
            object.name = entry.at("display_name").get<std::string>();
            object.kind = readKind(entry.at("kind").get<std::string>());
            object.visible = entry.at("visible").get<bool>();
            object.geometryPath = entry.at("geometry_ref").get<std::string>();
            object.materialOverride = entry.at("material_override").get<bool>();
            const auto& transform = entry.at("transform");
            object.transform.position = readVector(transform.at("position"));
            object.transform.rotationDegrees = readVector(transform.at("rotation_degrees"));
            object.transform.scale = readVector(transform.at("scale"));
            const auto& material = entry.at("material");
            object.material.color = readVector(material.at("base_color"));
            object.material.ambient = readVector(material.at("ambient"));
            object.material.diffuse = readVector(material.at("diffuse"));
            object.material.specular = readVector(material.at("specular"));
            if (material.contains("emission"))
                object.material.emission = readVector(material.at("emission"));
            object.material.emissionStrength = material.value("emission_strength", 0.0F);
            object.material.shininess = material.at("shininess").get<float>();
            object.material.texturePath = material.at("texture_ref").get<std::string>();
            parsed.scene.objects.push_back(std::move(object));
        }
        for (const auto& entry : input.at("lights")) {
            Light light;
            light.id = entry.at("id").get<std::string>();
            light.type = readLightType(entry.at("type").get<std::string>());
            light.position = readVector(entry.at("position"));
            light.direction = readVector(entry.at("direction"));
            light.color = readVector(entry.at("color"));
            light.intensity = entry.at("intensity").get<float>();
            light.enabled = entry.at("enabled").get<bool>();
            parsed.scene.lights.push_back(std::move(light));
        }
        for (const auto& entry : input.at("resources")) {
            SceneResource resource;
            resource.relativePath = entry.at("relative_path").get<std::string>();
            resource.kind = readResourceKind(entry.at("kind").get<std::string>());
            parsed.resources.push_back(std::move(resource));
        }
        parsed.scene.selectedId = input.value("selected_id", std::string{});
        document = std::move(parsed);
        return {};
    } catch (const std::exception& error) {
        return Result::failure(std::string("Invalid scene document: ") + error.what());
    }
}
}
