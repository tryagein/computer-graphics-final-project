#include "scene/Transform.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
namespace scene_editor {
glm::mat4 Transform::matrix() const { glm::mat4 m(1); m=glm::translate(m,position); m=glm::rotate(m,glm::radians(rotationDegrees.x),{1,0,0}); m=glm::rotate(m,glm::radians(rotationDegrees.y),{0,1,0}); m=glm::rotate(m,glm::radians(rotationDegrees.z),{0,0,1}); return glm::scale(m,scale); }
bool Transform::valid() const { for(int i=0;i<3;++i) if(!std::isfinite(position[i])||!std::isfinite(rotationDegrees[i])||!std::isfinite(scale[i])||std::abs(scale[i])<1e-6F)return false; return true; }
}
