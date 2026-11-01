#pragma once
#include "renderer/ShaderProgram.hpp"
#include "renderer/Mesh.hpp"
#include "scene/Scene.hpp"
#include "scene/Camera.hpp"
#include "renderer/SceneRenderer.hpp"
namespace scene_editor { class Renderer { public: bool initialize();void shutdown();void resize(int,int);void draw(const Scene&,const Camera&);double frameSeconds()const{return frameSeconds_;}private:int width{1280},height{720};double frameSeconds_{};ShaderProgram shader;Mesh cube,sphere,plane;SceneRenderer sceneRenderer;}; }
