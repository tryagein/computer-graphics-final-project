#pragma once
#include "agent/ConversationContext.hpp"
#include "agent/AgentProvider.hpp"
#include "agent/PlanExecutor.hpp"
#include "scene/Scene.hpp"
#include "scene/Camera.hpp"
#include "renderer/Renderer.hpp"
#include <memory>
#include <mutex>

namespace scene_editor {
class AgentPanel {
public:
    void update(Scene& scene, Camera& camera, Renderer& renderer);
    void draw(Scene& scene, Camera& camera);
private:
    struct Pending {
        std::mutex mutex;
        bool done{};
        ProviderPlan result;
    };
    ConversationContext context_;
    PlanExecutor executor_;
    std::shared_ptr<Pending> pending_;
    char input_[4096]{};
    std::string status_;
    bool completionRecorded_{true};
};
}
