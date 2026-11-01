#include "renderer/Lighting.hpp"
#include <glm/gtc/type_ptr.hpp>
namespace scene_editor {
void uploadLights(const std::vector<Light>& lights,const ShaderProgram& s){int points=0,directions=0;for(const auto& l:lights){if(!l.enabled||!l.valid())continue;if(l.type==LightType::Point&&points<4){auto p="pointLights["+std::to_string(points++)+"]";glUniform3fv(s.uniform((p+".position").c_str()),1,glm::value_ptr(l.position));glUniform3fv(s.uniform((p+".color").c_str()),1,glm::value_ptr(l.color));glUniform1f(s.uniform((p+".intensity").c_str()),l.intensity);}else if(l.type==LightType::Directional&&directions<2){auto p="directionalLights["+std::to_string(directions++)+"]";glUniform3fv(s.uniform((p+".direction").c_str()),1,glm::value_ptr(l.direction));glUniform3fv(s.uniform((p+".color").c_str()),1,glm::value_ptr(l.color));glUniform1f(s.uniform((p+".intensity").c_str()),l.intensity);}}glUniform1i(s.uniform("pointCount"),points);glUniform1i(s.uniform("directionalCount"),directions);}
}
