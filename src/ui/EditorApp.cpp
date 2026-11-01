#include "ui/EditorApp.hpp"
#include "ui/SceneObjectPanel.hpp"
#include "ui/MaterialPanel.hpp"
#include "ui/LightPanel.hpp"
#include "ui/CameraControls.hpp"
#include "ui/SceneFilePanel.hpp"
#include "assets/AssetService.hpp"
#include "assets/PrefabCatalog.hpp"
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>

namespace scene_editor {
namespace {
constexpr ImGuiWindowFlags panelFlags = ImGuiWindowFlags_NoDecoration |
    ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
    ImGuiWindowFlags_NoSavedSettings;

void placePanel(const char* name, float x, float y, float w, float h, ImGuiWindowFlags extra = 0) {
    ImGui::SetNextWindowPos({x, y});
    ImGui::SetNextWindowSize({w, h});
    ImGui::Begin(name, nullptr, panelFlags | extra);
}

void drawImportDialog(Scene& scene, Notifications& notifications) {
    static char importPath[1024]{};
    static bool initializedPath = false;
    static std::string importError;
    if (!initializedPath) {
        std::snprintf(importPath, sizeof(importPath), "%s/examples/pyramid.obj", SCENE_EDITOR_ASSET_DIR);
        initializedPath = true;
    }
    if (!ImGui::BeginPopupModal("导入 OBJ 模型", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) return;
    ImGui::TextUnformatted("输入本地 OBJ 路径。默认已填入示例模型。");
    ImGui::SetNextItemWidth(540);
    if (ImGui::InputText("文件路径", importPath, sizeof(importPath))) importError.clear();
    if (ImGui::Button("导入", {100, 0})) {
        if (addImportedObj(scene, importPath, importError)) {
            notifications.set("OBJ 已导入，可在左侧对象列表中选择并编辑。");
            importError.clear();
            ImGui::CloseCurrentPopup();
        } else notifications.report(Result::failure(importError));
    }
    ImGui::SameLine();
    if (ImGui::Button("取消", {100, 0})) ImGui::CloseCurrentPopup();
    if (!importError.empty()) ImGui::TextWrapped("%s", importError.c_str());
    ImGui::EndPopup();
}
}

bool EditorApp::initialize(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    auto& io = ImGui::GetIO();
#ifdef _WIN32
    io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/msyh.ttc", 16.0F, nullptr,
                                 io.Fonts->GetGlyphRangesChineseFull());
#endif
    ImGui::StyleColorsDark();
    auto& style = ImGui::GetStyle();
    style.WindowRounding = 0;
    style.FrameRounding = 4;
    style.WindowBorderSize = 1;
    style.WindowPadding = {12, 10};
    style.ItemSpacing = {8, 8};
    style.Colors[ImGuiCol_WindowBg] = {0.105F, 0.115F, 0.135F, 1.0F};
    style.Colors[ImGuiCol_Button] = {0.18F, 0.29F, 0.42F, 1.0F};
    style.Colors[ImGuiCol_ButtonHovered] = {0.24F, 0.39F, 0.56F, 1.0F};
    style.Colors[ImGuiCol_Tab] = {0.18F, 0.22F, 0.28F, 1.0F};
    style.Colors[ImGuiCol_TabSelected] = {0.22F, 0.36F, 0.52F, 1.0F};
    style.Colors[ImGuiCol_TabHovered] = {0.28F, 0.43F, 0.60F, 1.0F};
    if (!ImGui_ImplGlfw_InitForOpenGL(window, true)) { ImGui::DestroyContext(); return false; }
    if (!ImGui_ImplOpenGL3_Init("#version 330")) {
        ImGui_ImplGlfw_Shutdown(); ImGui::DestroyContext(); return false;
    }
    active_ = true;
    return true;
}

void EditorApp::draw(Scene& scene, Renderer& renderer, Camera& camera, GLFWwindow* window, float dt) {
    if (!active_) return;
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    agent_.update(scene, camera, renderer);
    const ImGuiIO& io = ImGui::GetIO();
    const float w = io.DisplaySize.x;
    const float h = io.DisplaySize.y;
    const float toolbarHeight = 58.0F;
    const float footerHeight = 38.0F;
    const float leftWidth = std::clamp(w * 0.19F, 190.0F, 270.0F);
    const float rightWidth = std::clamp(w * 0.30F, 300.0F, 420.0F);
    const float centerWidth = std::max(1.0F, w - leftWidth - rightWidth);
    const float bodyHeight = std::max(1.0F, h - toolbarHeight - footerHeight);

    placePanel("##Toolbar", 0, 0, w, toolbarHeight, ImGuiWindowFlags_NoScrollbar);
    drawSceneFileButton();
    ImGui::SameLine(130);
    ImGui::TextUnformatted("添加");
    ImGui::SameLine();
    if (ImGui::Button("立方体", {88, 34})) addPrimitive(scene, ObjectKind::Cube, notifications_);
    ImGui::SameLine();
    if (ImGui::Button("球体", {88, 34})) addPrimitive(scene, ObjectKind::Sphere, notifications_);
    ImGui::SameLine();
    if (ImGui::Button("平面", {88, 34})) addPrimitive(scene, ObjectKind::Plane, notifications_);
    ImGui::SameLine();
    if (ImGui::Button("导入 OBJ", {110, 34})) ImGui::OpenPopup("导入 OBJ 模型");
    ImGui::SameLine();
    if (ImGui::Button("预设模型", {110, 34})) ImGui::OpenPopup("预设模型菜单");
    if (ImGui::BeginPopup("预设模型菜单")) {
        auto add = [&](const char* assetId) {
            const bool pine = std::string(assetId) == "snow_pine";
            const bool cabin = std::string(assetId) == "snow_cabin";
            const int count = int(std::count_if(scene.objects.begin(), scene.objects.end(),
                [&](const SceneObject& object) {
                    return object.name == (pine ? "Snow pine" : cabin ? "Snow cabin" : "Street lamp");
                }));
            const glm::vec3 position{-2.0F + 2.4F * float(count % 4), 0,
                                     (pine ? -1.0F : cabin ? -3.0F : 1.5F) + 2.4F * float(count / 4)};
            const auto result = addPrefab(scene, assetId, position);
            if (result.ok) notifications_.set("已添加预设模型，可在左侧选择。");
            else notifications_.report(Result::failure(result.message));
        };
        if (ImGui::MenuItem("积雪松树")) add("snow_pine");
        if (ImGui::MenuItem("暖光路灯")) add("street_lamp");
        if (ImGui::MenuItem("雪地木屋")) add("snow_cabin");
        ImGui::EndPopup();
    }
    ImGui::SameLine();
    if (ImGui::Button("重置相机", {110, 34})) {
        camera = Camera{};
        notifications_.set("相机已重置。");
    }
    drawSceneFileDialog(scene, camera, renderer, notifications_);
    drawImportDialog(scene, notifications_);
    ImGui::End();

    placePanel("##Scene", 0, toolbarHeight, leftWidth, bodyHeight);
    drawSceneObjectPanel(scene);
    ImGui::End();

    placePanel("##Properties", w - rightWidth, toolbarHeight, rightWidth, bodyHeight);
    if (ImGui::BeginTabBar("InspectorTabs")) {
        if (ImGui::BeginTabItem("属性")) {
            drawObjectInspector(scene);
            drawMaterialPanel(scene);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("灯光")) {
            drawLightPanel(scene, renderer);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Agent")) {
            agent_.draw(scene, camera);
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();

    placePanel("##Viewport", leftWidth, toolbarHeight, centerWidth, bodyHeight,
               ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::TextDisabled("3D 视口");
    ImGui::SameLine();
    ImGui::TextDisabled("右键拖动观察  |  WASD 移动  |  QE 升降");
    const ImVec2 available = ImGui::GetContentRegionAvail();
    const ImVec2 imageSize{std::max(1.0F, available.x), std::max(1.0F, available.y)};
    renderer.resize(std::max(1, int(std::lround(imageSize.x * io.DisplayFramebufferScale.x))),
                    std::max(1, int(std::lround(imageSize.y * io.DisplayFramebufferScale.y))));
    ImGui::Image((ImTextureID)(intptr_t)renderer.viewportTexture(), imageSize,
                 {0, 1}, {1, 0});
    const bool hovered = ImGui::IsItemHovered();
    if (hovered && (ImGui::IsMouseClicked(ImGuiMouseButton_Left) ||
                    ImGui::IsMouseClicked(ImGuiMouseButton_Right))) viewportFocused_ = true;
    if (!hovered && (ImGui::IsMouseClicked(ImGuiMouseButton_Left) ||
                     ImGui::IsMouseClicked(ImGuiMouseButton_Right))) viewportFocused_ = false;
    updateCameraControls(window, camera, dt, hovered, viewportFocused_);
    ImGui::End();

    placePanel("##Status", 0, h - footerHeight, w, footerHeight, ImGuiWindowFlags_NoScrollbar);
    ImGui::Text("对象 %d   灯光 %d   渲染 %.2f ms", int(scene.objects.size()),
                int(scene.lights.size()), renderer.frameSeconds() * 1000.0);
    ImGui::SameLine(360);
    notifications_.draw();
    ImGui::End();

    renderer.draw(scene, camera);
    int fbWidth = 1, fbHeight = 1;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, fbWidth, fbHeight);
    glDisable(GL_DEPTH_TEST);
    glClearColor(0.09F, 0.10F, 0.12F, 1.0F);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void EditorApp::shutdown() {
    if (!active_) return;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    active_ = false;
}
}
