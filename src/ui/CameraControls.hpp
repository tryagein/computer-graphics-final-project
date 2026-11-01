#pragma once
#include "scene/Camera.hpp"
struct GLFWwindow;
namespace scene_editor {void updateCameraControls(GLFWwindow*,Camera&,float dt,bool viewportHovered,bool viewportFocused);}
