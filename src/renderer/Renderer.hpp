#pragma once
#include "renderer/ShaderProgram.hpp"
#include "renderer/Mesh.hpp"
#include "scene/Scene.hpp"
#include "scene/Camera.hpp"
#include "renderer/SceneRenderer.hpp"
namespace scene_editor { class Renderer { public: bool initialize();void shutdown();void invalidateSceneResources(){sceneRenderer.shutdown();}void resize(int,int);void draw(const Scene&,const Camera&);double frameSeconds()const{return frameSeconds_;}int viewportWidth()const{return width;}int viewportHeight()const{return height;}GLuint viewportTexture()const{return colorTexture_;}private:int width{},height{};double frameSeconds_{};GLuint framebuffer_{},colorTexture_{},depthBuffer_{};ShaderProgram shader;Mesh cube,sphere,plane;SceneRenderer sceneRenderer;}; }
