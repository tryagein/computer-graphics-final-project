#pragma once
#include "app/Result.hpp"
#include "persistence/SceneDocument.hpp"
#include <nlohmann/json.hpp>

namespace scene_editor {
nlohmann::json serializeSceneDocument(const SceneDocument& document);
Result deserializeSceneDocument(const nlohmann::json& json, SceneDocument& document);
}
