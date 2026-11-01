#pragma once
#include "scene/Transform.hpp"
#include <glm/glm.hpp>
#include <string>
#include <vector>
namespace scene_editor {
enum class ObjectKind { Cube, Sphere, Plane, ImportedModel }; enum class LightType { Point, Directional };
struct Material { glm::vec3 color{0.8F}; glm::vec3 ambient{0.15F}; glm::vec3 diffuse{0.8F}; glm::vec3 specular{0.3F}; glm::vec3 emission{0}; float emissionStrength{0}; float shininess{32}; std::string texturePath; bool valid()const; };
struct SceneObject { std::string id,name; ObjectKind kind{ObjectKind::Cube}; Transform transform; Material material; std::string geometryPath; bool materialOverride{false}; bool visible{true}; bool valid()const; };
struct Light { std::string id; LightType type{LightType::Point}; glm::vec3 position{1,2,1},direction{-0.5F,-1,-0.3F},color{1}; float intensity{1}; bool enabled{true}; bool valid()const; };
struct SceneResource {std::string relativePath;enum class Kind{Model,MaterialLibrary,Texture} kind{Kind::Model};bool valid()const{return !relativePath.empty();}};
class Scene { public: std::vector<SceneObject> objects; std::vector<Light> lights; std::string selectedId; bool add(SceneObject); bool erase(const std::string&); SceneObject* find(const std::string&); void makeDefault(); };
}
