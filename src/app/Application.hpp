#pragma once
#include "renderer/Renderer.hpp"
#include "scene/Scene.hpp"
#include "scene/Camera.hpp"
#include "ui/EditorApp.hpp"
#include <string>
struct GLFWwindow;
namespace scene_editor { class Application { public: int run(int benchmarkSeconds = 0, const std::string& capturePath = {}, bool captureWithoutShadows = false, bool benchmarkPrefabs = false); private: GLFWwindow* window_{}; Scene scene_; Camera camera_; Renderer renderer_; EditorApp editor_; }; }
