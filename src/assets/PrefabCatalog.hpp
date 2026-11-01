#pragma once
#include "scene/Scene.hpp"
#include <string>

namespace scene_editor {
struct PrefabResult {
    bool ok{};
    std::string message;
    std::string objectId;
    std::string lightId;
};

// Asset IDs are fixed; callers cannot supply arbitrary model paths.
PrefabResult addPrefab(Scene& scene, const std::string& assetId,
                       const glm::vec3& position, const std::string& requestedId = {});
}
