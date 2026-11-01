#include "app/Application.hpp"
#include "persistence/SceneFileService.hpp"
#ifdef _WIN32
#include <Windows.h>
#endif
#include <GLFW/glfw3.h>
#include <glad/gl.h>
#include <iostream>
#include <string>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <vector>
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4996)
#endif
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

namespace scene_editor {
namespace {
int startupError(int code, const std::string& message) {
    std::cerr << message << '\n';
#ifdef _WIN32
    MessageBoxA(nullptr, message.c_str(), "OpenGL Scene Editor", MB_OK | MB_ICONERROR);
#endif
    return code;
}

void centerWindow(GLFWwindow* window, int width, int height) {
    glfwShowWindow(window);
    glfwRestoreWindow(window);
    if (auto* monitor = glfwGetPrimaryMonitor()) {
        int x = 0, y = 0, workWidth = 0, workHeight = 0;
        glfwGetMonitorWorkarea(monitor, &x, &y, &workWidth, &workHeight);
        glfwSetWindowPos(window, x + (workWidth - width) / 2, y + (workHeight - height) / 2);
    }
}

int benchmark(Scene& scene, Camera& camera, Renderer& renderer, EditorApp& editor,
              GLFWwindow* window, int seconds) {
    scene.objects.clear();
    scene.selectedId.clear();
    for (int row = 0; row < 10; ++row) {
        for (int column = 0; column < 10; ++column) {
            SceneObject object;
            object.id = "benchmark-" + std::to_string(row * 10 + column);
            object.name = object.id;
            object.kind = (row + column) % 2 == 0 ? ObjectKind::Cube : ObjectKind::Sphere;
            object.transform.position = {(column - 4.5F) * 2.1F, 0.7F, (row - 4.5F) * 2.1F};
            object.transform.scale = {0.8F, 0.8F, 0.8F};
            object.material.color = {0.2F + 0.06F * column, 0.3F + 0.04F * row, 0.65F};
            if (!scene.add(std::move(object))) return 1;
        }
    }
    const auto vendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
    const auto gpu = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
    const auto version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    std::vector<double> frameRates;
    frameRates.reserve(size_t(seconds) * 1000);
    const auto start = std::chrono::steady_clock::now();
    auto previous = start;
    double elapsed = 0;
    while (elapsed < seconds && !glfwWindowShouldClose(window)) {
        const auto frameStart = std::chrono::steady_clock::now();
        elapsed = std::chrono::duration<double>(frameStart - start).count();
        const float angle = float(elapsed * 0.45);
        const glm::vec3 position{27.0F * std::cos(angle), 7.0F, 27.0F * std::sin(angle)};
        const glm::vec3 direction = -position;
        const float yaw = glm::degrees(std::atan2(direction.z, direction.x));
        const float pitch = glm::degrees(std::asin(direction.y / glm::length(direction)));
        camera.setPose(position, yaw, pitch);
        glfwPollEvents();
        const float dt = float(std::chrono::duration<double>(frameStart - previous).count());
        editor.draw(scene, renderer, camera, window, dt);
        glFinish();
        glfwSwapBuffers(window);
        const auto frameEnd = std::chrono::steady_clock::now();
        const double delta = std::chrono::duration<double>(frameEnd - previous).count();
        previous = frameEnd;
        if (delta > 0 && elapsed > 1.0) frameRates.push_back(1.0 / delta);
    }
    if (frameRates.empty()) return 1;
    const auto middle = frameRates.begin() + frameRates.size() / 2;
    std::nth_element(frameRates.begin(), middle, frameRates.end());
    std::cout << "GPU vendor: " << (vendor ? vendor : "unknown") << '\n'
              << "GPU renderer: " << (gpu ? gpu : "unknown") << '\n'
              << "OpenGL version: " << (version ? version : "unknown") << '\n'
              << "Window resolution: 1280x720\n"
              << "Viewport resolution: " << renderer.viewportWidth() << 'x'
              << renderer.viewportHeight() << '\n'
              << "Objects: " << scene.objects.size() << '\n'
              << "Duration seconds: " << elapsed << '\n'
              << "Measured frames after warmup: " << frameRates.size() << '\n'
              << "Median FPS: " << *middle << '\n';
    return *middle >= 30.0 ? 0 : 2;
}

int captureDemo(Scene& scene, Camera& camera, Renderer& renderer, const std::string& outputPath) {
    const auto demo = std::filesystem::path(SCENE_EDITOR_ASSET_DIR) / "examples" /
                      "course-demo" / "course-demo.json";
    const auto loaded = loadSceneFile(demo, scene, camera);
    if (!loaded.ok) { std::cerr << loaded.message << '\n'; return 1; }
    constexpr int width = 1280, height = 720;
    renderer.resize(width, height);
    renderer.draw(scene, camera);
    GLuint readFramebuffer = 0;
    glGenFramebuffers(1, &readFramebuffer);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, readFramebuffer);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           renderer.viewportTexture(), 0);
    if (glCheckFramebufferStatus(GL_READ_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        glDeleteFramebuffers(1, &readFramebuffer);
        std::cerr << "Demo screenshot framebuffer is incomplete.\n";
        return 1;
    }
    std::vector<unsigned char> pixels(size_t(width) * height * 4);
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &readFramebuffer);
    for (int y = 0; y < height / 2; ++y) {
        const auto top = pixels.begin() + size_t(y) * width * 4;
        const auto bottom = pixels.begin() + size_t(height - 1 - y) * width * 4;
        std::swap_ranges(top, top + width * 4, bottom);
    }
    if (!stbi_write_png(outputPath.c_str(), width, height, 4, pixels.data(), width * 4)) {
        std::cerr << "Could not write demo screenshot: " << outputPath << '\n';
        return 1;
    }
    std::cout << "Captured demo scene with " << scene.objects.size() << " objects to "
              << outputPath << '\n';
    return 0;
}
}

int Application::run(int benchmarkSeconds, const std::string& capturePath) {
    if (!glfwInit()) return startupError(1, "GLFW initialization failed. Check the graphics driver.");
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    window_ = glfwCreateWindow(1280, 720, "OpenGL Scene Editor", nullptr, nullptr);
    if (!window_) {
        glfwTerminate();
        return startupError(2, "Could not create an OpenGL 3.3 window. Check the graphics driver.");
    }
    glfwSetWindowSizeLimits(window_, 900, 600, GLFW_DONT_CARE, GLFW_DONT_CARE);
    glfwMakeContextCurrent(window_);
    if (!gladLoadGL(glfwGetProcAddress)) {
        glfwDestroyWindow(window_);
        glfwTerminate();
        return startupError(3, "OpenGL function loading failed.");
    }
    glfwSwapInterval(1);
    scene_.makeDefault();
    scene_.selectedId = "cube-1";
    if (!renderer_.initialize()) {
        renderer_.shutdown();
        glfwDestroyWindow(window_);
        glfwTerminate();
        return startupError(4, "Shader initialization failed. Check the project assets directory.");
    }
    if (!capturePath.empty()) {
        const int result = captureDemo(scene_, camera_, renderer_, capturePath);
        renderer_.shutdown();
        glfwDestroyWindow(window_);
        glfwTerminate();
        return result;
    }
    if (!editor_.initialize(window_)) {
        renderer_.shutdown();
        glfwDestroyWindow(window_);
        glfwTerminate();
        return startupError(5, "Editor interface initialization failed.");
    }
    centerWindow(window_, 1280, 720);
    if (benchmarkSeconds) {
        const int result = benchmark(scene_, camera_, renderer_, editor_, window_, benchmarkSeconds);
        editor_.shutdown();
        renderer_.shutdown();
        glfwDestroyWindow(window_);
        glfwTerminate();
        return result;
    }
    double last = glfwGetTime();
    while (!glfwWindowShouldClose(window_)) {
        glfwPollEvents();
        const double now = glfwGetTime();
        const float dt = float(now - last);
        last = now;
        editor_.draw(scene_, renderer_, camera_, window_, dt);
        glfwSwapBuffers(window_);
    }
    editor_.shutdown();
    renderer_.shutdown();
    glfwDestroyWindow(window_);
    glfwTerminate();
    return 0;
}
}
