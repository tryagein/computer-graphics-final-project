#include "scene/Scene.hpp"
#include <algorithm>
#include <cmath>
namespace scene_editor {
static bool finite(const glm::vec3& v){return std::isfinite(v.x)&&std::isfinite(v.y)&&std::isfinite(v.z);}
bool Material::valid()const{return finite(color)&&finite(ambient)&&finite(diffuse)&&finite(specular)&&std::isfinite(shininess)&&shininess>0;}
bool SceneObject::valid()const{return !id.empty()&&transform.valid()&&material.valid()&&(kind!=ObjectKind::ImportedModel||!geometryPath.empty());}
bool Light::valid()const{return !id.empty()&&finite(position)&&finite(direction)&&finite(color)&&std::isfinite(intensity)&&intensity>=0&&(type!=LightType::Directional||glm::length(direction)>1e-6F);}
bool Scene::add(SceneObject o){if(!o.valid()||find(o.id))return false;objects.push_back(std::move(o));return true;}
bool Scene::erase(const std::string&id){auto n=objects.size();objects.erase(std::remove_if(objects.begin(),objects.end(),[&](const auto&o){return o.id==id;}),objects.end());if(selectedId==id)selectedId.clear();return n!=objects.size();}
SceneObject* Scene::find(const std::string&id){auto i=std::find_if(objects.begin(),objects.end(),[&](const auto&o){return o.id==id;});return i==objects.end()?nullptr:&*i;}
void Scene::makeDefault(){objects.clear();lights.clear();selectedId.clear();SceneObject floor;floor.id="plane-1";floor.name="Ground";floor.kind=ObjectKind::Plane;floor.transform.scale={8,1,8};floor.material.color={0.55F,0.58F,0.62F};add(floor);SceneObject cube;cube.id="cube-1";cube.name="Cube";cube.transform.position={0,0.5F,0};add(cube);Light sun;sun.id="directional-1";sun.type=LightType::Directional;lights.push_back(sun);Light lamp;lamp.id="point-1";lamp.position={2,3,2};lamp.color={1,0.78F,0.5F};lights.push_back(lamp);}
}
