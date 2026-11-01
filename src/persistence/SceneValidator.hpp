#pragma once
#include "app/Result.hpp"
#include "persistence/SceneDocument.hpp"
#include <filesystem>

namespace scene_editor {
// Validates every reference before changing the document to absolute runtime paths.
Result validateSceneDocument(SceneDocument& document, const std::filesystem::path& sceneFile);
}
