#include "scene/Camera.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>
namespace scene_editor {
void Camera::rotate(float dx,float dy){yaw_+=dx;pitch_=std::clamp(pitch_+dy,-89.0F,89.0F);}
bool Camera::setPose(glm::vec3 position,float yaw,float pitch){if(!std::isfinite(position.x)||!std::isfinite(position.y)||!std::isfinite(position.z)||!std::isfinite(yaw)||!std::isfinite(pitch)||pitch<=-90||pitch>=90)return false;eye_=position;yaw_=yaw;pitch_=pitch;return true;}
bool Camera::setProjection(float fovDegrees,float nearPlane,float farPlane){if(!std::isfinite(fovDegrees)||!std::isfinite(nearPlane)||!std::isfinite(farPlane)||fovDegrees<=1||fovDegrees>=179||nearPlane<=0||farPlane<=nearPlane)return false;fovDegrees_=fovDegrees;nearPlane_=nearPlane;farPlane_=farPlane;return true;}
glm::mat4 Camera::view()const{glm::vec3 f{std::cos(glm::radians(yaw_))*std::cos(glm::radians(pitch_)),std::sin(glm::radians(pitch_)),std::sin(glm::radians(yaw_))*std::cos(glm::radians(pitch_))};return glm::lookAt(eye_,eye_+glm::normalize(f),{0,1,0});}
glm::mat4 Camera::projection(float aspect)const{return glm::perspective(glm::radians(fovDegrees_),std::isfinite(aspect)&&aspect>0?aspect:1.0F,nearPlane_,farPlane_);}
}
