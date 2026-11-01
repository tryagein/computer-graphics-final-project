#include "renderer/Renderer.hpp"
#include "renderer/PrimitiveMeshes.hpp"
#include "renderer/Lighting.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <GLFW/glfw3.h>
namespace scene_editor {
bool Renderer::initialize(){cube=makeCube();sphere=makeSphere();plane=makePlane();return shader.load(std::string(SCENE_EDITOR_ASSET_DIR)+"/shaders/phong.vert",std::string(SCENE_EDITOR_ASSET_DIR)+"/shaders/phong.frag");}
void Renderer::shutdown(){sceneRenderer.shutdown();cube.reset();sphere.reset();plane.reset();shader.reset();}
void Renderer::resize(int w,int h){width=w>0?w:1;height=h>0?h:1;glViewport(0,0,w,h);}
void Renderer::draw(const Scene&s,const Camera&c){const double begin=glfwGetTime();glEnable(GL_DEPTH_TEST);glClearColor(.06F,.07F,.1F,1);glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);shader.use();auto view=c.view(),projection=c.projection(float(width)/float(height));glUniformMatrix4fv(shader.uniform("view"),1,GL_FALSE,glm::value_ptr(view));glUniformMatrix4fv(shader.uniform("projection"),1,GL_FALSE,glm::value_ptr(projection));glm::vec3 vp=c.position();glUniform3fv(shader.uniform("viewPos"),1,glm::value_ptr(vp));uploadLights(s.lights,shader);sceneRenderer.draw(s,shader,cube,sphere,plane);frameSeconds_=glfwGetTime()-begin;}
}
