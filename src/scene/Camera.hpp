#pragma once
#include <glm/glm.hpp>
namespace scene_editor { class Camera { public: glm::vec3 position()const{return eye_;}void move(glm::vec3 d){eye_+=d;}void rotate(float,float);bool setProjection(float fovDegrees,float nearPlane,float farPlane);glm::mat4 view()const;glm::mat4 projection(float aspect)const;private:glm::vec3 eye_{0,2,6};float yaw_{-90},pitch_{-12};float fovDegrees_{45},nearPlane_{0.1F},farPlane_{500};}; }
