#pragma once
#include <string>
#include <vector>

namespace scene_editor {
struct ConversationTurn { std::string role; std::string content; };
class ConversationContext {
public:
    void add(std::string role, std::string content);
    void clear() { turns_.clear(); }
    const std::vector<ConversationTurn>& turns() const { return turns_; }
    std::string promptText() const;
private:
    std::vector<ConversationTurn> turns_;
};
}
