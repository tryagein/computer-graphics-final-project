#pragma once
#include "renderer/ShaderProgram.hpp"
#include "renderer/Mesh.hpp"
#include "scene/Scene.hpp"
#include "scene/Camera.hpp"
#include "renderer/SceneRenderer.hpp"
namespace scene_editor {
class Renderer {
public:
    bool initialize();
    void shutdown();
    void invalidateSceneResources() { sceneRenderer.shutdown(); }
    void resize(int, int);
    void draw(const Scene&, const Camera&);
    double frameSeconds() const { return frameSeconds_; }
    int viewportWidth() const { return width; }
    int viewportHeight() const { return height; }
    GLuint viewportTexture() const { return colorTexture_; }
    bool shadowsEnabled() const { return shadowsEnabled_; }
    void setShadowsEnabled(bool value) { shadowsEnabled_ = value; }
    float shadowBias() const { return shadowBias_; }
    void setShadowBias(float value) { shadowBias_ = value; }
    int shadowPcfRadius() const { return shadowPcfRadius_; }
    void setShadowPcfRadius(int value) { shadowPcfRadius_ = value; }
    bool shadowAvailable() const { return shadowAvailable_; }
private:
    int width{}, height{};
    double frameSeconds_{};
    GLuint framebuffer_{}, colorTexture_{}, depthBuffer_{};
    GLuint shadowFramebuffer_{}, shadowTexture_{};
    bool shadowAvailable_{}, shadowsEnabled_{true};
    float shadowBias_{0.003F};
    int shadowPcfRadius_{1};
    ShaderProgram shader, depthShader;
    Mesh cube, sphere, plane;
    SceneRenderer sceneRenderer;
};
}
