#include "persistence/SceneFileService.hpp"
#include "assets/PrefabCatalog.hpp"
#include <filesystem>
#include <iostream>
#include <string>

using namespace scene_editor;

namespace {
SceneObject primitive(std::string id, std::string name, ObjectKind kind,
                      glm::vec3 position, glm::vec3 scale, glm::vec3 color) {
    SceneObject object;
    object.id = std::move(id);
    object.name = std::move(name);
    object.kind = kind;
    object.transform.position = position;
    object.transform.scale = scale;
    object.material.color = color;
    return object;
}
}

int main() {
    namespace fs = std::filesystem;
    const fs::path folder = fs::path(SCENE_EDITOR_ASSET_DIR) / "examples" / "course-demo";
    const fs::path prefabs = fs::path(SCENE_EDITOR_ASSET_DIR) / "prefabs";
    Scene scene;
    auto ground = primitive("snow-ground", "Snow ground", ObjectKind::Plane,
                            {0, 0, 0}, {12, 1, 12}, {0.8F, 0.87F, 0.94F});
    ground.material.texturePath = (prefabs / "snow-grain.png").string();
    scene.add(std::move(ground));
    for (int i = 0; i < 3; ++i) {
        const float x = -3.0F + 2.5F * i;
        const float z = -1.0F + (i % 2 == 0 ? -0.5F : 0.8F);
        const auto added = addPrefab(scene, "snow_pine", {x, 0, z},
                                     "tree-" + std::to_string(i + 1));
        if (!added.ok) { std::cerr << added.message << '\n'; return 1; }
    }
    const auto lamp = addPrefab(scene, "street_lamp", {2.7F, 0, 0}, "warm-lamp");
    if (!lamp.ok) { std::cerr << lamp.message << '\n'; return 1; }
    const auto cabin = addPrefab(scene, "snow_cabin", {-1.2F, 0, -4.0F}, "snow-cabin");
    if (!cabin.ok) { std::cerr << cabin.message << '\n'; return 1; }
    SceneObject crate = primitive("textured-crate", "Textured OBJ crate", ObjectKind::ImportedModel,
                                  {0.7F, 0, 2.1F}, {0.9F, 0.9F, 0.9F}, {1, 1, 1});
    crate.geometryPath = (folder / "textured-crate.obj").string();
    scene.add(std::move(crate));
    scene.selectedId = "textured-crate";

    Light moon;
    moon.id = "moonlight";
    moon.type = LightType::Directional;
    moon.direction = {-0.4F, -1.0F, -0.3F};
    moon.color = {0.4F, 0.55F, 0.9F};
    moon.intensity = 0.65F;
    scene.lights.push_back(moon);

    Camera camera;
    if (!camera.setPose({6, 3.2F, 8}, -128, -12)) return 1;
    const fs::path file = folder / "course-demo.json";
    const auto saved = saveSceneFile(file, scene, camera);
    if (!saved.ok) { std::cerr << saved.message << '\n'; return 1; }
    Scene restored;
    Camera restoredCamera;
    const auto loaded = loadSceneFile(file, restored, restoredCamera);
    if (!loaded.ok || restored.objects.size() != scene.objects.size()) {
        std::cerr << "Generated demo failed load validation: " << loaded.message << '\n';
        return 1;
    }
    std::cout << "Generated " << file << " with " << restored.objects.size()
              << " objects and " << restored.lights.size() << " lights.\n";
    return 0;
}
