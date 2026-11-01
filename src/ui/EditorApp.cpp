#include "ui/EditorApp.hpp"
#include "ui/SceneObjectPanel.hpp"
#include "ui/MaterialPanel.hpp"
#include "ui/LightPanel.hpp"
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
namespace scene_editor {
bool EditorApp::initialize(GLFWwindow* window){IMGUI_CHECKVERSION();ImGui::CreateContext();ImGui::StyleColorsDark();if(!ImGui_ImplGlfw_InitForOpenGL(window,true)){ImGui::DestroyContext();return false;}if(!ImGui_ImplOpenGL3_Init("#version 330")){ImGui_ImplGlfw_Shutdown();ImGui::DestroyContext();return false;}active_=true;return true;}
void EditorApp::draw(Scene& scene,const Renderer& renderer){if(!active_)return;ImGui_ImplOpenGL3_NewFrame();ImGui_ImplGlfw_NewFrame();ImGui::NewFrame();drawSceneObjectPanel(scene,notifications_);drawMaterialPanel(scene);drawLightPanel(scene);ImGui::Begin("Status");ImGui::Text("Objects: %d  Lights: %d",int(scene.objects.size()),int(scene.lights.size()));ImGui::Text("Render: %.2f ms",renderer.frameSeconds()*1000.0);notifications_.draw();ImGui::End();ImGui::Render();ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());}
void EditorApp::shutdown(){if(!active_)return;ImGui_ImplOpenGL3_Shutdown();ImGui_ImplGlfw_Shutdown();ImGui::DestroyContext();active_=false;}
}
