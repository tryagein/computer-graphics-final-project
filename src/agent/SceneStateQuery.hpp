#pragma once
#include "scene/Scene.hpp"
#include "scene/Camera.hpp"
#include <nlohmann/json.hpp>
#include <string>

namespace scene_editor {
struct ObjectResolution { SceneObject* object{}; std::string error; };
ObjectResolution resolveSceneObject(Scene& scene, const std::string& idOrName);
nlohmann::json compactSceneState(const Scene& scene, const Camera& camera);
}
