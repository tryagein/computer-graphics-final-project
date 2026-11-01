#pragma once
#include "agent/OperationCodec.hpp"
#include "agent/SceneOperationDispatcher.hpp"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace scene_editor {
struct PlanStepResult { SceneOperation operation; OperationResult result; };
class PlanExecutor {
public:
    Result load(const nlohmann::json& plan);
    void step(Scene& scene, Camera& camera);
    bool finished() const { return done_ || next_ >= operations_.size(); }
    bool stoppedOnError() const { return stopped_; }
    bool resourcesChanged() const { return resourcesChanged_; }
    size_t total() const { return operations_.size(); }
    size_t completed() const { return next_; }
    const std::string& summary() const { return summary_; }
    const std::vector<PlanStepResult>& results() const { return results_; }
private:
    std::string summary_;
    std::vector<SceneOperation> operations_;
    std::vector<PlanStepResult> results_;
    size_t next_{};
    bool done_{true};
    bool stopped_{};
    bool resourcesChanged_{};
};
}
