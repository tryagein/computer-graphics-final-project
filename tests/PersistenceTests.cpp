#include "persistence/SceneFileService.hpp"
#include "persistence/SceneSerializer.hpp"
#include <nlohmann/json.hpp>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace fs = std::filesystem;
using namespace scene_editor;

namespace {
void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

void write(const fs::path& path, const std::string& content) {
    std::ofstream stream(path, std::ios::binary);
    stream << content;
    require(bool(stream), "fixture write failed");
}

nlohmann::json readJson(const fs::path& path) {
    std::ifstream stream(path);
    return nlohmann::json::parse(stream);
}
}

int main() {
    const auto nonce = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    const fs::path root = fs::temp_directory_path() / ("scene-persistence-test-" + nonce);
    try {
        const fs::path source = root / "source";
        const fs::path saved = root / "saved";
        const fs::path moved = root / "moved";
        fs::create_directories(source);
        fs::create_directories(saved);
        write(source / "model.obj",
              "mtllib model.mtl\no Test\nv 0 0 0\nv 1 0 0\nv 0 1 0\n"
              "vt 0 0\nvt 1 0\nvt 0 1\nusemtl Test\nf 1/1 2/2 3/3\n");
        write(source / "model.mtl", "newmtl Test\nKd 0.7 0.4 0.2\nmap_Kd color.png\n");
        write(source / "color.png", "texture-fixture");

        Scene scene;
        SceneObject cube;
        cube.id = "cube-A";
        cube.name = "First cube";
        cube.transform.position = {1, 2, 3};
        cube.material.color = {0.2F, 0.5F, 0.8F};
        cube.material.emission = {1.0F, 0.7F, 0.3F};
        cube.material.emissionStrength = 1.5F;
        require(scene.add(cube), "could not add cube");
        SceneObject model;
        model.id = "model-A";
        model.name = "Imported model";
        model.kind = ObjectKind::ImportedModel;
        model.geometryPath = (source / "model.obj").string();
        model.material.texturePath = (source / "color.png").string();
        model.materialOverride = true;
        require(scene.add(model), "could not add model");
        scene.selectedId = model.id;
        Light light;
        light.id = "light-A";
        light.type = LightType::Directional;
        light.direction = {-1, -1, 0};
        light.intensity = 2;
        scene.lights.push_back(light);
        Camera camera;
        require(camera.setPose({4, 5, 6}, 30, -20), "camera pose rejected");
        require(camera.setProjection(60, 0.2F, 700), "camera projection rejected");

        const fs::path file = saved / "my-scene.json";
        auto result = saveSceneFile(file, scene, camera);
        require(result.ok, result.message.c_str());
        const auto json = readJson(file);
        require(json.at("schema_version") == 1, "schema version missing");
        require(json.at("objects").size() == 2, "objects not serialized");
        require(json.at("resources").size() >= 4, "OBJ dependencies not packaged");
        require(!fs::path(json.at("objects").at(1).at("geometry_ref").get<std::string>()).is_absolute(),
                "saved geometry reference is absolute");

        Scene restored;
        Camera restoredCamera;
        result = loadSceneFile(file, restored, restoredCamera);
        require(result.ok, result.message.c_str());
        require(restored.objects.size() == 2 && restored.lights.size() == 1,
                "scene entity counts changed after load");
        require(restored.selectedId == "model-A", "selection did not round-trip");
        require(restored.find("cube-A")->transform.position.x == 1, "transform did not round-trip");
        require(restored.find("cube-A")->material.color.y == 0.5F, "material did not round-trip");
        require(restored.find("cube-A")->material.emissionStrength == 1.5F &&
                restored.find("cube-A")->material.emission.y == 0.7F,
                "emissive material did not round-trip");
        require(restoredCamera.position().x == 4 && restoredCamera.yaw() == 30 &&
                restoredCamera.fovDegrees() == 60, "camera did not round-trip");
        require(fs::exists(restored.find("model-A")->geometryPath), "OBJ path did not resolve");
        require(fs::exists(restored.find("model-A")->material.texturePath), "texture path did not resolve");
        auto legacy = json;
        for (auto& object : legacy["objects"]) {
            object["material"].erase("emission");
            object["material"].erase("emission_strength");
        }
        const fs::path legacyFile = saved / "legacy-scene.json";
        write(legacyFile, legacy.dump());
        Scene legacyScene;
        Camera legacyCamera;
        result = loadSceneFile(legacyFile, legacyScene, legacyCamera);
        require(result.ok && legacyScene.find("cube-A") &&
                legacyScene.find("cube-A")->material.emissionStrength == 0,
                "older scene without emission did not load");

        fs::rename(saved, moved);
        const fs::path movedFile = moved / "my-scene.json";
        result = loadSceneFile(movedFile, restored, restoredCamera);
        require(result.ok, result.message.c_str());

        const fs::path unicodeFolder = root / fs::u8path("自选保存位置");
        fs::create_directories(unicodeFolder);
        const fs::path unicodeFile = unicodeFolder / fs::u8path("夜晚场景.json");
        result = saveSceneFile(unicodeFile, scene, camera);
        require(result.ok, result.message.c_str());
        require(fs::exists(unicodeFolder / fs::u8path("夜晚场景.resources")),
                "resource directory was not created beside chosen scene file");
        Scene unicodeRestored;
        Camera unicodeCamera;
        result = loadSceneFile(unicodeFile, unicodeRestored, unicodeCamera);
        require(result.ok && unicodeRestored.objects.size() == 2,
                "scene in a selected Unicode folder did not reopen");

        const std::string preservedJson = readJson(movedFile).dump();
        restored.find("cube-A")->material.color.x = 2.0F;
        result = saveSceneFile(movedFile, restored, restoredCamera);
        require(!result.ok && readJson(movedFile).dump() == preservedJson,
                "invalid save replaced the previous scene file");
        restored.find("cube-A")->material.color.x = 0.2F;
        require(fs::exists(restored.find("model-A")->geometryPath), "moved OBJ did not resolve");
        result = saveSceneFile(movedFile, restored, restoredCamera);
        require(result.ok, result.message.c_str());
        result = loadSceneFile(movedFile, restored, restoredCamera);
        require(result.ok, result.message.c_str());

        const fs::path malformed = moved / "malformed.json";
        write(malformed, "{not valid json");
        const std::string beforeId = restored.selectedId;
        result = loadSceneFile(malformed, restored, restoredCamera);
        require(!result.ok && restored.selectedId == beforeId && restored.objects.size() == 2 &&
                restoredCamera.position().x == 4,
                "malformed load changed the active scene");

        auto broken = readJson(movedFile);
        broken["schema_version"] = 999;
        write(malformed, broken.dump());
        result = loadSceneFile(malformed, restored, restoredCamera);
        require(!result.ok && restored.selectedId == beforeId, "unsupported version changed the scene");

        broken = readJson(movedFile);
        broken["resources"][0]["relative_path"] = "../outside.obj";
        write(malformed, broken.dump());
        result = loadSceneFile(malformed, restored, restoredCamera);
        require(!result.ok && restored.selectedId == beforeId, "unsafe path changed the scene");

        const auto missingTexture = fs::path(restored.find("model-A")->material.texturePath);
        fs::remove(missingTexture);
        result = loadSceneFile(movedFile, restored, restoredCamera);
        require(!result.ok && restored.selectedId == beforeId && restored.objects.size() == 2,
                "missing resource changed the active scene");
        fs::remove_all(root);
        std::cout << "scene persistence tests passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        std::error_code ignored;
        fs::remove_all(root, ignored);
        return 1;
    }
}
