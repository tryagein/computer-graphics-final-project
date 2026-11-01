#include "agent/SceneStateQuery.hpp"
#include <glm/glm.hpp>

namespace scene_editor {
namespace {
nlohmann::json vec(const glm::vec3& v) { return {v.x, v.y, v.z}; }
const char* kind(ObjectKind value) {
    switch (value) {
    case ObjectKind::Cube: return "cube";
    case ObjectKind::Sphere: return "sphere";
    case ObjectKind::Plane: return "plane";
    case ObjectKind::ImportedModel: return "imported_model";
    }
    return "unknown";
}
}

ObjectResolution resolveSceneObject(Scene& scene, const std::string& idOrName) {
    if (idOrName.empty()) return {nullptr, "Target object is missing."};
    if (auto* exact = scene.find(idOrName)) return {exact, {}};
    SceneObject* match = nullptr;
    for (auto& object : scene.objects) {
        if (object.name != idOrName) continue;
        if (match) return {nullptr, "Ambiguous object name: " + idOrName};
        match = &object;
    }
    if (!match) return {nullptr, "Object not found: " + idOrName};
    return {match, {}};
}

nlohmann::json compactSceneState(const Scene& scene, const Camera& camera) {
    nlohmann::json result = {
        {"objects", nlohmann::json::array()}, {"lights", nlohmann::json::array()},
        {"camera", {{"position", vec(camera.position())},
                    {"yaw", camera.yaw()}, {"pitch", camera.pitch()}}}
    };
    for (const auto& object : scene.objects)
        result["objects"].push_back({{"id", object.id}, {"name", object.name},
            {"kind", kind(object.kind)}, {"position", vec(object.transform.position)},
            {"visible", object.visible}});
    for (const auto& light : scene.lights)
        result["lights"].push_back({{"id", light.id},
            {"type", light.type == LightType::Point ? "point" : "directional"},
            {"enabled", light.enabled}});
    return result;
}
}
