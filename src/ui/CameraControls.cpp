#include "ui/CameraControls.hpp"
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <glm/gtc/matrix_transform.hpp>
namespace scene_editor {
void updateCameraControls(GLFWwindow* window,Camera& camera,float dt,bool viewportHovered,bool viewportFocused){
    static bool dragging=false;static double lastX=0,lastY=0;
    if(!viewportFocused||ImGui::GetIO().WantTextInput){dragging=false;return;}
    const float speed=4.0F*dt;
    glm::mat4 inv=glm::inverse(camera.view());glm::vec3 forward=glm::normalize(glm::vec3(inv[2])*-1.0F);glm::vec3 right=glm::normalize(glm::vec3(inv[0]));
    if(glfwGetKey(window,GLFW_KEY_W)==GLFW_PRESS)camera.move(forward*speed);
    if(glfwGetKey(window,GLFW_KEY_S)==GLFW_PRESS)camera.move(-forward*speed);
    if(glfwGetKey(window,GLFW_KEY_A)==GLFW_PRESS)camera.move(-right*speed);
    if(glfwGetKey(window,GLFW_KEY_D)==GLFW_PRESS)camera.move(right*speed);
    if(glfwGetKey(window,GLFW_KEY_Q)==GLFW_PRESS)camera.move({0,-speed,0});
    if(glfwGetKey(window,GLFW_KEY_E)==GLFW_PRESS)camera.move({0,speed,0});
    if(viewportHovered&&glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_RIGHT)==GLFW_PRESS){double x,y;glfwGetCursorPos(window,&x,&y);if(dragging)camera.rotate(float(x-lastX)*0.12F,float(lastY-y)*0.12F);lastX=x;lastY=y;dragging=true;}else dragging=false;
}
}
