#include "ui/AgentPanel.hpp"
#include "agent/SceneStateQuery.hpp"
#include <imgui.h>
#include <nlohmann/json.hpp>
#include <algorithm>
#include <thread>

namespace scene_editor {
void AgentPanel::update(Scene& scene, Camera& camera, Renderer& renderer) {
    if (pending_) {
        ProviderPlan completed;
        bool ready = false;
        {
            std::lock_guard<std::mutex> lock(pending_->mutex);
            if (pending_->done) { completed = std::move(pending_->result); ready = true; }
        }
        if (ready) {
            pending_.reset();
            if (!completed.status.ok) {
                status_ = completed.status.message;
                context_.add("assistant", status_);
            } else {
                const auto loaded = executor_.load(completed.plan);
                if (!loaded.ok) {
                    status_ = loaded.message;
                    context_.add("assistant", status_);
                } else {
                    status_ = executor_.summary();
                    completionRecorded_ = false;
                }
            }
        }
    }

    if (!pending_ && !executor_.finished()) {
        const auto before = executor_.results().size();
        executor_.step(scene, camera);
        for (size_t i = before; i < executor_.results().size(); ++i)
            if (executor_.results()[i].result.resourcesChanged) renderer.invalidateSceneResources();
    }
    if (!completionRecorded_ && executor_.finished()) {
        const auto& results = executor_.results();
        const size_t successful = std::count_if(results.begin(), results.end(),
            [](const PlanStepResult& step) { return step.result.ok && step.operation.tool != "get_scene_state"; });
        const auto failed = std::find_if(results.begin(), results.end(),
            [](const PlanStepResult& step) { return !step.result.ok; });
        status_ = executor_.stoppedOnError() ? "执行已停止：" + failed->result.message
                                              : "执行完成：" + std::to_string(successful) + " 个操作成功。";
        context_.add("assistant", executor_.summary() + " " + status_ +
            " Current scene: " + compactSceneState(scene, camera).dump());
        completionRecorded_ = true;
    }
}

void AgentPanel::draw(Scene& scene, Camera& camera) {
    const auto config = ProviderConfig::fromEnvironment();
    ImGui::TextWrapped("描述想创建或修改的场景。Agent 只调用已验证的场景工具。");
    ImGui::TextDisabled("%s", config.ready() ? "自然语言服务已配置" :
        "自然语言需设置 OPENAI_API_KEY 和 OPENAI_MODEL");
    ImGui::SeparatorText("对话");
    if (ImGui::BeginChild("AgentHistory", {0, 150}, ImGuiChildFlags_Borders)) {
        for (const auto& turn : context_.turns()) {
            ImGui::TextDisabled("%s", turn.role == "user" ? "用户" : "Agent");
            ImGui::TextWrapped("%s", turn.content.c_str());
            ImGui::Spacing();
        }
    }
    ImGui::EndChild();

    ImGui::InputTextMultiline("##AgentPrompt", input_, sizeof(input_), {-1, 90});
    const bool busy = pending_ || !executor_.finished();
    ImGui::BeginDisabled(busy);
    if (ImGui::Button("生成并执行", {130, 32})) {
        const std::string request(input_);
        if (request.find_first_not_of(" \t\r\n") == std::string::npos) {
            status_ = "请先输入场景要求。";
        } else {
            input_[0] = 0;
            ConversationContext history = context_;
            context_.add("user", request);
            if (request.front() == '{') {
                try {
                    const auto loaded = executor_.load(nlohmann::json::parse(request));
                    status_ = loaded.ok ? executor_.summary() : loaded.message;
                    completionRecorded_ = !loaded.ok;
                } catch (const std::exception& error) {
                    status_ = std::string("JSON 计划无效：") + error.what();
                    completionRecorded_ = true;
                }
            } else if (!config.ready()) {
                status_ = "请先配置模型服务，或粘贴结构化 JSON 计划进行离线验收。";
                context_.add("assistant", status_);
            } else {
                const auto snapshot = compactSceneState(scene, camera);
                pending_ = std::make_shared<Pending>();
                const auto state = pending_;
                std::thread([state, config, request, history, snapshot]() mutable {
                    ProviderPlan result;
                    try { result = OpenAIProvider(config).plan(request, history, snapshot); }
                    catch (const std::exception& error) {
                        result.status = Result::failure(std::string("Model request failed: ") + error.what());
                    }
                    std::lock_guard<std::mutex> lock(state->mutex);
                    state->result = std::move(result);
                    state->done = true;
                }).detach();
                status_ = "正在规划，请稍候…";
            }
        }
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    if (ImGui::Button("清空会话", {110, 32})) {
        context_.clear();
        executor_ = PlanExecutor{};
        pending_.reset();
        completionRecorded_ = true;
        status_ = "会话已清空。";
    }
    if (!status_.empty()) ImGui::TextWrapped("%s", status_.c_str());
    if (!executor_.results().empty() || !executor_.finished()) {
        ImGui::SeparatorText("操作进度");
        ImGui::Text("%d / %d", int(executor_.completed()), int(executor_.total()));
        for (const auto& step : executor_.results()) {
            const std::string label = step.operation.operationId + "  " + step.operation.tool;
            if (ImGui::TreeNode(label.c_str())) {
                ImGui::TextWrapped("%s: %s", step.result.ok ? "成功" : "失败",
                                   step.result.message.c_str());
                if (!step.result.data.empty()) {
                    const auto details = step.result.data.dump(2);
                    ImGui::TextWrapped("%s", details.c_str());
                }
                ImGui::TreePop();
            }
        }
    }
    ImGui::TextDisabled("可粘贴 JSON 计划离线测试；示例见 docs/agent-setup.md");
}
}
