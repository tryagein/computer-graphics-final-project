#pragma once
#include "scene/Scene.hpp"
#include "ui/Notifications.hpp"
namespace scene_editor {bool addPrimitive(Scene& scene,ObjectKind kind,Notifications& notifications);void drawSceneObjectPanel(Scene& scene);void drawObjectInspector(Scene& scene);}
