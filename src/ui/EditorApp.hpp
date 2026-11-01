#pragma once
#include "scene/Scene.hpp"
#include "renderer/Renderer.hpp"
#include "scene/Camera.hpp"
#include "ui/Notifications.hpp"
#include "ui/AgentPanel.hpp"
struct GLFWwindow;
namespace scene_editor {class EditorApp {public:bool initialize(GLFWwindow*);void draw(Scene&,Renderer&,Camera&,GLFWwindow*,float dt);void shutdown();Notifications& notifications(){return notifications_;}private:Notifications notifications_;AgentPanel agent_;bool active_{};bool viewportFocused_{};};}
