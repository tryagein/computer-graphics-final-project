#include "assets/PrefabCatalog.hpp"
#include "assets/ObjImporter.hpp"
#include <algorithm>
#include <cmath>
#include <filesystem>

namespace scene_editor {
namespace {
bool used(const Scene& scene, const std::string& id) {
    return std::any_of(scene.objects.begin(), scene.objects.end(),
        [&](const SceneObject& object) { return object.id == id; }) ||
        std::any_of(scene.lights.begin(), scene.lights.end(),
        [&](const Light& light) { return light.id == id; });
}
}

PrefabResult addPrefab(Scene& scene, const std::string& assetId,
                       const glm::vec3& position, const std::string& requestedId) {
    if (assetId != "snow_pine" && assetId != "street_lamp" && assetId != "snow_cabin")
        return {false, "Unknown prefab asset ID: " + assetId};
    for (int i = 0; i < 3; ++i)
        if (!std::isfinite(position[i])) return {false, "Prefab position must be finite."};
    const bool lamp = assetId == "street_lamp";
    std::string id = requestedId;
    if (id.empty()) {
        for (unsigned i = 1; i < 100000; ++i) {
            id = assetId + "-" + std::to_string(i);
            if (!used(scene, id) && (!lamp || !used(scene, id + "-light"))) break;
        }
    }
    if (id.empty() || id.size() > 256 || used(scene, id) ||
        (lamp && used(scene, id + "-light")))
        return {false, "Prefab ID already exists or is invalid."};
    if (lamp && std::count_if(scene.lights.begin(), scene.lights.end(),
        [](const Light& light) { return light.type == LightType::Point; }) >= 4)
        return {false, "Point-light limit reached; cannot add street lamp."};

    const auto file = std::filesystem::path(SCENE_EDITOR_ASSET_DIR) / "prefabs" /
        (lamp ? "street-lamp.obj" : assetId == "snow_cabin" ? "snow-cabin.obj" : "snow-pine.obj");
    const auto imported = importObj(file.string());
    if (!imported.valid()) return {false, "Prefab OBJ could not be loaded: " + imported.error};
    SceneObject object;
    object.id = id;
    object.name = lamp ? "Street lamp" : assetId == "snow_cabin" ? "Snow cabin" : "Snow pine";
    object.kind = ObjectKind::ImportedModel;
    object.geometryPath = file.string();
    object.transform.position = position;
    object.material = imported.submeshes.front().material;
    if (!scene.add(object)) return {false, "Could not add prefab object."};
    std::string lightId;
    if (lamp) {
        Light light;
        light.id = id + "-light";
        light.position = position + glm::vec3{0.72F, 2.85F, 0};
        light.color = {1.0F, 0.72F, 0.4F};
        light.intensity = 2.2F;
        lightId = light.id;
        scene.lights.push_back(light);
    }
    scene.selectedId = id;
    return {true, "Prefab added.", id, lightId};
}
}
