#pragma once
#include <glm/glm.hpp>
namespace scene_editor { // Right-handed world: +Y up, camera looks toward -Z at yaw -90; rotations are X then Y then Z in degrees.
struct Transform { glm::vec3 position{0}; glm::vec3 rotationDegrees{0}; glm::vec3 scale{1}; glm::mat4 matrix() const; bool valid() const; }; }
