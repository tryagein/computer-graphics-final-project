#pragma once
#include "scene/Scene.hpp"
#include "scene/Camera.hpp"
#include "renderer/Renderer.hpp"
#include "ui/Notifications.hpp"

namespace scene_editor {
void drawSceneFileButton();
void drawSceneFileDialog(Scene& scene, Camera& camera, Renderer& renderer,
                         Notifications& notifications);
}
