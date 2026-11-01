#pragma once
#include "app/Result.hpp"
#include "agent/ConversationContext.hpp"
#include <nlohmann/json.hpp>
#include <string>

namespace scene_editor {
struct ProviderConfig {
    std::string apiKey;
    std::string model;
    std::string proxy;
    static ProviderConfig fromEnvironment();
    bool ready() const { return !apiKey.empty() && !model.empty(); }
};

struct ProviderPlan {
    Result status;
    nlohmann::json plan;
};

class AgentProvider {
public:
    virtual ~AgentProvider() = default;
    virtual ProviderPlan plan(const std::string& request,
                              const ConversationContext& context,
                              const nlohmann::json& sceneState) = 0;
};

class DeepSeekProvider final : public AgentProvider {
public:
    explicit DeepSeekProvider(ProviderConfig config) : config_(std::move(config)) {}
    ProviderPlan plan(const std::string& request, const ConversationContext& context,
                      const nlohmann::json& sceneState) override;
private:
    ProviderConfig config_;
};

ProviderPlan parseDeepSeekPlanResponse(const nlohmann::json& response);
}
