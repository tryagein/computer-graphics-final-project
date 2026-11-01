#include "renderer/Renderer.hpp"
#include "renderer/PrimitiveMeshes.hpp"
#include "renderer/Lighting.hpp"
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>
#include <iostream>

namespace scene_editor {
namespace {
constexpr int shadowResolution = 2048;
}

bool Renderer::initialize() {
    cube = makeCube();
    sphere = makeSphere();
    plane = makePlane();
    const std::string assets = SCENE_EDITOR_ASSET_DIR;
    if (!shader.load(assets + "/shaders/phong.vert", assets + "/shaders/phong.frag") ||
        !depthShader.load(assets + "/shaders/shadow-depth.vert",
                          assets + "/shaders/shadow-depth.frag")) return false;

    glGenFramebuffers(1, &shadowFramebuffer_);
    glGenTextures(1, &shadowTexture_);
    glBindTexture(GL_TEXTURE_2D, shadowTexture_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, shadowResolution, shadowResolution,
                 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    constexpr float border[] = {1, 1, 1, 1};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFramebuffer_);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowTexture_, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    shadowAvailable_ = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    if (!shadowAvailable_) std::cerr << "Shadow framebuffer unavailable; rendering without shadows.\n";
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return true;
}

void Renderer::shutdown() {
    sceneRenderer.shutdown();
    cube.reset(); sphere.reset(); plane.reset();
    shader.reset(); depthShader.reset();
    if (shadowTexture_) glDeleteTextures(1, &shadowTexture_);
    if (shadowFramebuffer_) glDeleteFramebuffers(1, &shadowFramebuffer_);
    if (depthBuffer_) glDeleteRenderbuffers(1, &depthBuffer_);
    if (colorTexture_) glDeleteTextures(1, &colorTexture_);
    if (framebuffer_) glDeleteFramebuffers(1, &framebuffer_);
    shadowTexture_ = shadowFramebuffer_ = 0;
    depthBuffer_ = colorTexture_ = framebuffer_ = 0;
    shadowAvailable_ = false;
    width = height = 0;
}

void Renderer::resize(int w, int h) {
    w = w > 0 ? w : 1;
    h = h > 0 ? h : 1;
    if (w == width && h == height && framebuffer_) return;
    width = w; height = h;
    if (!framebuffer_) glGenFramebuffers(1, &framebuffer_);
    if (!colorTexture_) glGenTextures(1, &colorTexture_);
    if (!depthBuffer_) glGenRenderbuffers(1, &depthBuffer_);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);
    glBindTexture(GL_TEXTURE_2D, colorTexture_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture_, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer_);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                              GL_RENDERBUFFER, depthBuffer_);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "Viewport framebuffer creation failed\n";
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::draw(const Scene& scene, const Camera& camera) {
    const double begin = glfwGetTime();
    const Light* shadowLight = nullptr;
    if (shadowsEnabled_ && shadowAvailable_) {
        for (const auto& light : scene.lights) {
            if (light.enabled && light.valid() && light.type == LightType::Directional) {
                shadowLight = &light;
                break;
            }
        }
    }
    glm::mat4 lightSpace{1};
    if (shadowLight) {
        const glm::vec3 direction = glm::normalize(shadowLight->direction);
        const glm::vec3 up = std::abs(direction.y) > 0.98F ?
            glm::vec3{0, 0, 1} : glm::vec3{0, 1, 0};
        lightSpace = glm::ortho(-20.0F, 20.0F, -20.0F, 20.0F, 1.0F, 75.0F) *
            glm::lookAt(-direction * 30.0F, glm::vec3{0}, up);
        glBindFramebuffer(GL_FRAMEBUFFER, shadowFramebuffer_);
        glViewport(0, 0, shadowResolution, shadowResolution);
        glEnable(GL_DEPTH_TEST);
        glClear(GL_DEPTH_BUFFER_BIT);
        depthShader.use();
        glUniformMatrix4fv(depthShader.uniform("lightSpaceMatrix"), 1, GL_FALSE,
                           glm::value_ptr(lightSpace));
        sceneRenderer.drawDepth(scene, depthShader, cube, sphere, plane);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);
    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST);
    glClearColor(.06F, .07F, .1F, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shader.use();
    const auto view = camera.view();
    const auto projection = camera.projection(float(width) / float(height));
    glUniformMatrix4fv(shader.uniform("view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(shader.uniform("projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(shader.uniform("lightSpaceMatrix"), 1, GL_FALSE,
                       glm::value_ptr(lightSpace));
    glUniform1i(shader.uniform("shadowEnabled"), shadowLight ? 1 : 0);
    glUniform1f(shader.uniform("shadowBias"), shadowBias_);
    glUniform1i(shader.uniform("shadowPcfRadius"), shadowPcfRadius_);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, shadowTexture_);
    glUniform1i(shader.uniform("shadowMap"), 1);
    glActiveTexture(GL_TEXTURE0);
    const glm::vec3 viewPosition = camera.position();
    glUniform3fv(shader.uniform("viewPos"), 1, glm::value_ptr(viewPosition));
    uploadLights(scene.lights, shader);
    sceneRenderer.draw(scene, shader, cube, sphere, plane);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    frameSeconds_ = glfwGetTime() - begin;
}
}
