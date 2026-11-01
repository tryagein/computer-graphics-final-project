#include "ui/Notifications.hpp"
#include <imgui.h>
namespace scene_editor {void Notifications::draw()const{if(!message_.empty())ImGui::TextWrapped("%s",message_.c_str());}}
