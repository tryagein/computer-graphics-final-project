#include "persistence/SceneFileService.hpp"
#include "persistence/ResourcePackager.hpp"
#include "persistence/SceneDocument.hpp"
#include "persistence/SceneSerializer.hpp"
#include "persistence/SceneValidator.hpp"
#include <chrono>
#include <fstream>
#include <set>

namespace scene_editor {
namespace {
namespace fs = std::filesystem;

bool normalizedColor(const glm::vec3& color) {
    for (int i = 0; i < 3; ++i)
        if (color[i] < 0 || color[i] > 1) return false;
    return true;
}

Result validateSaveSource(const Scene& scene) {
    std::set<std::string> ids;
    for (const auto& object : scene.objects) {
        if (!object.valid() || !ids.insert(object.id).second)
            return Result::failure("Cannot save an invalid or duplicate object: " + object.id);
        const auto& material = object.material;
        if (!normalizedColor(material.color) || !normalizedColor(material.ambient) ||
            !normalizedColor(material.diffuse) || !normalizedColor(material.specular))
            return Result::failure("Cannot save an out-of-range material color: " + object.id);
    }
    for (const auto& light : scene.lights)
        if (!light.valid() || !ids.insert(light.id).second || !normalizedColor(light.color))
            return Result::failure("Cannot save an invalid or duplicate light: " + light.id);
    if (!scene.selectedId.empty()) {
        bool found = false;
        for (const auto& object : scene.objects) found |= object.id == scene.selectedId;
        if (!found) return Result::failure("Selected object is missing from the scene.");
    }
    return {};
}

void discard(const fs::path& path) {
    std::error_code ignored;
    fs::remove_all(path, ignored);
}
}

Result saveSceneFile(const std::filesystem::path& path, const Scene& scene, const Camera& camera) {
    if (path.empty()) return Result::failure("Choose a scene JSON file path.");
    auto valid = validateSaveSource(scene);
    if (!valid.ok) return valid;
    Camera cameraCheck;
    if (!cameraCheck.setPose(camera.position(), camera.yaw(), camera.pitch()) ||
        !cameraCheck.setProjection(camera.fovDegrees(), camera.nearPlane(), camera.farPlane()))
        return Result::failure("Cannot save an invalid camera.");
    const fs::path file = fs::absolute(path);
    const fs::path parent = file.parent_path();
    const std::string resourceName = file.stem().string() + ".resources";
    const fs::path resources = parent / resourceName;
    const auto nonce = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    const fs::path stagedResources = parent / ("." + resourceName + ".stage-" + nonce);
    const fs::path stagedFile = parent / ("." + file.filename().string() + ".stage-" + nonce);
    const fs::path oldResources = parent / ("." + resourceName + ".backup-" + nonce);
    const fs::path oldFile = parent / ("." + file.filename().string() + ".backup-" + nonce);
    bool oldResourcesMoved = false, oldFileMoved = false;
    bool newResourcesMoved = false, newFileMoved = false;
    try {
        fs::create_directories(parent);
        SceneDocument document;
        document.scene = scene;
        document.camera = camera;
        auto packaged = packageSceneResources(document, stagedResources, resourceName);
        if (!packaged.ok) { discard(stagedResources); return packaged; }
        {
            std::ofstream output(stagedFile, std::ios::binary);
            if (!output) throw std::runtime_error("Cannot create scene file: " + stagedFile.string());
            output << serializeSceneDocument(document).dump(2) << '\n';
            if (!output) throw std::runtime_error("Failed to write scene JSON.");
        }
        if (fs::exists(resources)) { fs::rename(resources, oldResources); oldResourcesMoved = true; }
        if (fs::exists(file)) { fs::rename(file, oldFile); oldFileMoved = true; }
        fs::rename(stagedResources, resources); newResourcesMoved = true;
        fs::rename(stagedFile, file); newFileMoved = true;
        discard(oldResources);
        discard(oldFile);
        return {true, "Scene saved: " + file.string()};
    } catch (const std::exception& error) {
        if (newFileMoved) discard(file);
        if (newResourcesMoved) discard(resources);
        try { if (oldFileMoved) fs::rename(oldFile, file); } catch (...) {}
        try { if (oldResourcesMoved) fs::rename(oldResources, resources); } catch (...) {}
        discard(stagedFile);
        discard(stagedResources);
        return Result::failure(std::string("Scene save failed: ") + error.what());
    }
}

Result loadSceneFile(const std::filesystem::path& path, Scene& scene, Camera& camera) {
    if (path.empty()) return Result::failure("Choose a scene JSON file path.");
    try {
        const fs::path file = fs::absolute(path);
        std::error_code error;
        if (!fs::is_regular_file(file, error))
            return Result::failure("Scene file does not exist: " + file.string());
        if (fs::file_size(file) > 16 * 1024 * 1024)
            return Result::failure("Scene JSON exceeds the 16 MB limit.");
        std::ifstream input(file, std::ios::binary);
        if (!input) return Result::failure("Cannot read scene file: " + file.string());
        const auto json = nlohmann::json::parse(input);
        SceneDocument candidate;
        auto parsed = deserializeSceneDocument(json, candidate);
        if (!parsed.ok) return parsed;
        auto validated = validateSceneDocument(candidate, file);
        if (!validated.ok) return validated;
        scene = std::move(candidate.scene);
        camera = candidate.camera;
        return {true, "Scene loaded: " + file.string()};
    } catch (const std::exception& error) {
        return Result::failure(std::string("Scene load failed: ") + error.what());
    }
}
}
