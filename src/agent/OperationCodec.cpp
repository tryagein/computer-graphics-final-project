#include "agent/OperationCodec.hpp"
#include <set>

namespace scene_editor {
Result decodeOperation(const nlohmann::json& value, SceneOperation& operation) {
    static const std::set<std::string> allowed = {
        "add_object", "add_prefab", "update_object", "delete_object", "set_material",
        "add_light", "update_light", "delete_light", "move_camera",
        "get_scene_state", "render_scene", "save_scene", "load_scene"
    };
    if (!value.is_object() || value.size() != 3 || !value.contains("operation_id") ||
        !value.contains("tool") || !value.contains("arguments") ||
        !value.at("operation_id").is_string() || !value.at("tool").is_string() ||
        !value.at("arguments").is_object())
        return Result::failure("Operation must contain operation_id, tool, and object arguments only.");
    const auto id = value.at("operation_id").get<std::string>();
    const auto tool = value.at("tool").get<std::string>();
    if (id.empty() || id.size() > 64) return Result::failure("Invalid operation_id length.");
    if (!allowed.count(tool)) return Result::failure("Unsupported operation: " + tool);
    operation = {id, tool, value.at("arguments")};
    return {};
}

nlohmann::json encodeOperationResult(const SceneOperation& operation, bool ok,
                                     const std::string& message, const nlohmann::json& data) {
    return { {"operation_id", operation.operationId}, {"tool", operation.tool},
             {"ok", ok}, {"message", message}, {"data", data} };
}
}
