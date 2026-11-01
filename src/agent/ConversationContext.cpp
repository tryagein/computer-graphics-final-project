#include "agent/ConversationContext.hpp"

namespace scene_editor {
void ConversationContext::add(std::string role, std::string content) {
    if (role != "user" && role != "assistant") return;
    if (content.size() > 2000) content.resize(2000);
    turns_.push_back({std::move(role), std::move(content)});
    if (turns_.size() > 12) turns_.erase(turns_.begin(), turns_.begin() + (turns_.size() - 12));
}

std::string ConversationContext::promptText() const {
    std::string result;
    for (const auto& turn : turns_) {
        result += turn.role;
        result += ": ";
        result += turn.content;
        result += '\n';
    }
    return result;
}
}
