#include "agent/PlanExecutor.hpp"
#include "agent/SceneStateQuery.hpp"
#include <set>

namespace scene_editor {
Result PlanExecutor::load(const nlohmann::json& plan) {
    if (!plan.is_object() || !plan.contains("summary") || !plan.at("summary").is_string() ||
        !plan.contains("operations") || !plan.at("operations").is_array())
        return Result::failure("Plan needs a summary and operations array.");
    const auto& raw = plan.at("operations");
    if (raw.size() > 32) return Result::failure("Plan exceeds 32 operations.");
    std::vector<SceneOperation> parsed;
    std::set<std::string> ids;
    for (const auto& item : raw) {
        nlohmann::json envelope = item;
        if (envelope.is_object() && envelope.contains("arguments_json")) {
            try {
                if (envelope.contains("arguments") || !envelope.at("arguments_json").is_string())
                    return Result::failure("Plan has conflicting argument fields.");
                envelope["arguments"] = nlohmann::json::parse(envelope.at("arguments_json").get<std::string>());
                envelope.erase("arguments_json");
            } catch (const std::exception& error) {
                return Result::failure(std::string("Invalid arguments_json: ") + error.what());
            }
        }
        SceneOperation operation;
        auto result = decodeOperation(envelope, operation);
        if (!result.ok) return result;
        if (!ids.insert(operation.operationId).second)
            return Result::failure("Duplicate operation_id: " + operation.operationId);
        parsed.push_back(std::move(operation));
    }
    summary_ = plan.at("summary").get<std::string>();
    if (summary_.size() > 1000) summary_.resize(1000);
    operations_ = std::move(parsed);
    results_.clear();
    next_ = 0;
    done_ = operations_.empty();
    stopped_ = false;
    resourcesChanged_ = false;
    return {};
}

void PlanExecutor::step(Scene& scene, Camera& camera) {
    if (finished()) return;
    const auto& operation = operations_[next_++];
    auto result = dispatchSceneOperation(operation, scene, camera);
    resourcesChanged_ |= result.resourcesChanged;
    const bool unresolved = result.unresolvedTarget;
    const bool failed = !result.ok;
    results_.push_back({operation, std::move(result)});
    if (unresolved) {
        SceneOperation query{"recovery-" + operation.operationId, "get_scene_state", nlohmann::json::object()};
        results_.push_back({query, dispatchSceneOperation(query, scene, camera)});
    }
    if (failed) { done_ = true; stopped_ = true; }
    else if (next_ >= operations_.size()) done_ = true;
}
}
