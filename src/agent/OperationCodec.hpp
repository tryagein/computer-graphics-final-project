#pragma once
#include "app/Result.hpp"
#include <nlohmann/json.hpp>
#include <string>

namespace scene_editor {
struct SceneOperation {
    std::string operationId;
    std::string tool;
    nlohmann::json arguments;
};

Result decodeOperation(const nlohmann::json& value, SceneOperation& operation);
nlohmann::json encodeOperationResult(const SceneOperation& operation, bool ok,
                                     const std::string& message,
                                     const nlohmann::json& data = nlohmann::json::object());
}
