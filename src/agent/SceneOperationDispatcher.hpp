#pragma once
#include "agent/OperationCodec.hpp"
#include "scene/Scene.hpp"
#include "scene/Camera.hpp"
#include <nlohmann/json.hpp>

namespace scene_editor {
struct OperationResult {
    bool ok{};
    bool unresolvedTarget{};
    bool resourcesChanged{};
    std::string message;
    nlohmann::json data = nlohmann::json::object();
};
OperationResult dispatchSceneOperation(const SceneOperation& operation, Scene& scene, Camera& camera);
}
