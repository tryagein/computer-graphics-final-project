#pragma once
#include "scene/Scene.hpp"
#include "scene/Camera.hpp"
#include <vector>

namespace scene_editor {
inline constexpr int kSceneSchemaVersion = 1;

struct SceneDocument {
    int schemaVersion{kSceneSchemaVersion};
    Scene scene;
    Camera camera;
    std::vector<SceneResource> resources;
};
}
