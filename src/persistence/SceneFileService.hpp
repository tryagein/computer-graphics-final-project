#pragma once
#include "app/Result.hpp"
#include "scene/Scene.hpp"
#include "scene/Camera.hpp"
#include <filesystem>

namespace scene_editor {
Result saveSceneFile(const std::filesystem::path& path, const Scene& scene, const Camera& camera);
Result loadSceneFile(const std::filesystem::path& path, Scene& scene, Camera& camera);
}
