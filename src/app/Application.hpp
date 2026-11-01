#pragma once
#include "renderer/Renderer.hpp"
#include "scene/Scene.hpp"
#include "scene/Camera.hpp"
#include "ui/EditorApp.hpp"
struct GLFWwindow;
namespace scene_editor { class Application { public: int run(); private: GLFWwindow* window_{}; Scene scene_; Camera camera_; Renderer renderer_; EditorApp editor_; }; }
