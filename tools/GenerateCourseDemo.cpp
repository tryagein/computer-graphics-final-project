#include "persistence/SceneFileService.hpp"
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
    Scene scene;
    scene.add(primitive("snow-ground", "Snow ground", ObjectKind::Plane,
                        {0, 0, 0}, {12, 1, 12}, {0.8F, 0.87F, 0.94F}));
    for (int i = 0; i < 3; ++i) {
        const float x = -3.0F + 2.5F * i;
        const float z = -1.0F + (i % 2 == 0 ? -0.5F : 0.8F);
        scene.add(primitive("tree-trunk-" + std::to_string(i), "Tree trunk " + std::to_string(i + 1),
                            ObjectKind::Cube, {x, 0.7F, z}, {0.25F, 1.4F, 0.25F},
                            {0.35F, 0.22F, 0.13F}));
        scene.add(primitive("tree-crown-" + std::to_string(i), "Tree crown " + std::to_string(i + 1),
                            ObjectKind::Sphere, {x, 1.65F, z}, {0.8F, 1.05F, 0.8F},
                            {0.11F, 0.33F, 0.27F}));
    }
    scene.add(primitive("lamp-post", "Street lamp post", ObjectKind::Cube,
                        {2.7F, 1.35F, 0}, {0.12F, 2.7F, 0.12F}, {0.22F, 0.25F, 0.31F}));
    scene.add(primitive("lamp-head", "Warm street lamp", ObjectKind::Sphere,
                        {2.7F, 2.8F, 0}, {0.28F, 0.28F, 0.28F}, {1.0F, 0.75F, 0.36F}));
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
    Light warm;
    warm.id = "warm-lamp";
    warm.type = LightType::Point;
    warm.position = {2.7F, 2.8F, 0};
    warm.color = {1, 0.72F, 0.4F};
    warm.intensity = 2.2F;
    scene.lights.push_back(warm);

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
