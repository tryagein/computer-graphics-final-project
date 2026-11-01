#include "ui/SceneFilePanel.hpp"
#include "persistence/SceneFileService.hpp"
#include <imgui.h>
#include <cstdio>
#include <filesystem>
#include <string>

namespace scene_editor {
void drawSceneFileButton() {
    if (ImGui::Button("场景文件", {100, 34})) ImGui::OpenPopup("保存或打开场景");
}

void drawSceneFileDialog(Scene& scene, Camera& camera, Renderer& renderer,
                         Notifications& notifications) {
    static char path[1024]{};
    static bool initialized = false;
    static std::string lastError;
    if (!initialized) {
        const auto suggested = (std::filesystem::current_path() / "my-scene.json").string();
        std::snprintf(path, sizeof(path), "%s", suggested.c_str());
        initialized = true;
    }
    if (!ImGui::BeginPopupModal("保存或打开场景", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) return;
    ImGui::TextWrapped("JSON 文件与同名资源目录需要一起移动。打开场景会替换当前内容。");
    ImGui::SetNextItemWidth(560);
    if (ImGui::InputText("场景路径", path, sizeof(path))) lastError.clear();
    if (ImGui::Button("保存当前场景", {145, 34})) {
        const auto result = saveSceneFile(path, scene, camera);
        notifications.set(result.ok ? std::string("场景已保存：") + path
                                    : std::string("保存失败：") + result.message);
        lastError = result.ok ? "" : "保存失败：" + result.message;
        if (result.ok) ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    if (ImGui::Button("打开场景", {115, 34})) {
        const auto result = loadSceneFile(path, scene, camera);
        notifications.set(result.ok ? std::string("场景已打开：") + path
                                    : std::string("打开失败：") + result.message);
        lastError = result.ok ? "" : "打开失败：" + result.message;
        if (result.ok) {
            renderer.invalidateSceneResources();
            ImGui::CloseCurrentPopup();
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("取消", {90, 34})) ImGui::CloseCurrentPopup();
    if (!lastError.empty()) ImGui::TextWrapped("%s", lastError.c_str());
    ImGui::EndPopup();
}
}
