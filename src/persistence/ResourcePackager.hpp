#pragma once
#include "app/Result.hpp"
#include "persistence/SceneDocument.hpp"
#include <filesystem>
#include <string>

namespace scene_editor {
Result packageSceneResources(SceneDocument& document,
                             const std::filesystem::path& stagingDirectory,
                             const std::string& finalDirectoryName);
}
