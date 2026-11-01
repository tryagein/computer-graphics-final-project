#include "app/Application.hpp"
#include <GLFW/glfw3.h>
#include <glad/gl.h>
#include "ui/CameraControls.hpp"
#include <iostream>
namespace scene_editor {
int Application::run(){if(!glfwInit()){std::cerr<<"GLFW init failed\n";return 1;}glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);window_=glfwCreateWindow(1280,720,"OpenGL Scene Editor",nullptr,nullptr);if(!window_){glfwTerminate();return 2;}glfwMakeContextCurrent(window_);if(!gladLoadGL(glfwGetProcAddress)){std::cerr<<"GLAD load failed\n";glfwDestroyWindow(window_);glfwTerminate();return 3;}glfwSwapInterval(1);scene_.makeDefault();if(!renderer_.initialize()){std::cerr<<"Shader init failed\n";renderer_.shutdown();glfwDestroyWindow(window_);glfwTerminate();return 4;}if(!editor_.initialize(window_)){renderer_.shutdown();glfwDestroyWindow(window_);glfwTerminate();return 5;}double last=glfwGetTime();while(!glfwWindowShouldClose(window_)){glfwPollEvents();double now=glfwGetTime();updateCameraControls(window_,camera_,float(now-last));last=now;int w=1,h=1;glfwGetFramebufferSize(window_,&w,&h);renderer_.resize(w,h);renderer_.draw(scene_,camera_);editor_.draw(scene_,renderer_);glfwSwapBuffers(window_);}editor_.shutdown();renderer_.shutdown();glfwDestroyWindow(window_);glfwTerminate();return 0;}
}
