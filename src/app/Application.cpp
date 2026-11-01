#include "app/Application.hpp"
#ifdef _WIN32
#include <Windows.h>
#endif
#include <GLFW/glfw3.h>
#include <glad/gl.h>
#include <iostream>
#include <string>

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
}

int Application::run() {
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
    if (!editor_.initialize(window_)) {
        renderer_.shutdown();
        glfwDestroyWindow(window_);
        glfwTerminate();
        return startupError(5, "Editor interface initialization failed.");
    }
    centerWindow(window_, 1280, 720);
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
