#pragma once
#include "scene/Scene.hpp"
#include "renderer/Renderer.hpp"
#include "ui/Notifications.hpp"
struct GLFWwindow;
namespace scene_editor {class EditorApp {public:bool initialize(GLFWwindow*);void draw(Scene&,const Renderer&);void shutdown();Notifications& notifications(){return notifications_;}private:Notifications notifications_;bool active_{};};}
