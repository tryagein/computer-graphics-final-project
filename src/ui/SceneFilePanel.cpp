#include "ui/SceneFilePanel.hpp"
#include "persistence/SceneFileService.hpp"
#include <imgui.h>
#include <cstdio>
#include <filesystem>
#include <string>

#ifdef _WIN32
#include <Windows.h>
#include <objbase.h>
#include <shlobj.h>
#include <shobjidl.h>
#endif

namespace scene_editor {
namespace {
namespace fs = std::filesystem;

enum class PickResult { Selected, Cancelled, Failed };

#ifdef _WIN32
PickResult pickPath(bool folder, const fs::path& initialDirectory,
                    fs::path& selected, std::string& error) {
    const HRESULT init = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (FAILED(init) && init != RPC_E_CHANGED_MODE) {
        error = "无法初始化系统文件选择器。";
        return PickResult::Failed;
    }
    const bool uninitialize = SUCCEEDED(init);
    IFileOpenDialog* dialog = nullptr;
    HRESULT result = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER,
                                      IID_PPV_ARGS(&dialog));
    if (FAILED(result)) {
        if (uninitialize) CoUninitialize();
        error = "无法打开系统文件选择器。";
        return PickResult::Failed;
    }
    DWORD options = 0;
    result = dialog->GetOptions(&options);
    if (SUCCEEDED(result))
        result = dialog->SetOptions(options | FOS_FORCEFILESYSTEM | FOS_PATHMUSTEXIST |
                                    (folder ? FOS_PICKFOLDERS : FOS_FILEMUSTEXIST));
    if (SUCCEEDED(result)) result = dialog->SetTitle(folder ? L"选择场景保存文件夹" : L"打开场景文件");
    if (!folder && SUCCEEDED(result)) {
        const COMDLG_FILTERSPEC filters[] = {{L"场景文件 (*.json)", L"*.json"},
                                              {L"所有文件 (*.*)", L"*.*"}};
        result = dialog->SetFileTypes(2, filters);
    }
    if (!initialDirectory.empty() && SUCCEEDED(result)) {
        IShellItem* initial = nullptr;
        if (SUCCEEDED(SHCreateItemFromParsingName(initialDirectory.c_str(), nullptr,
                                                 IID_PPV_ARGS(&initial)))) {
            dialog->SetFolder(initial);
            initial->Release();
        }
    }
    if (SUCCEEDED(result)) result = dialog->Show(GetActiveWindow());
    if (result == HRESULT_FROM_WIN32(ERROR_CANCELLED)) {
        dialog->Release();
        if (uninitialize) CoUninitialize();
        return PickResult::Cancelled;
    }
    if (SUCCEEDED(result)) {
        IShellItem* item = nullptr;
        result = dialog->GetResult(&item);
        if (SUCCEEDED(result)) {
            PWSTR rawPath = nullptr;
            result = item->GetDisplayName(SIGDN_FILESYSPATH, &rawPath);
            if (SUCCEEDED(result)) {
                selected = fs::path(rawPath);
                CoTaskMemFree(rawPath);
            }
            item->Release();
        }
    }
    dialog->Release();
    if (uninitialize) CoUninitialize();
    if (FAILED(result)) {
        error = "无法读取所选路径，请重试。";
        return PickResult::Failed;
    }
    return PickResult::Selected;
}
#else
PickResult pickPath(bool, const fs::path&, fs::path&, std::string& error) {
    error = "当前平台暂不支持系统文件选择器。";
    return PickResult::Failed;
}
#endif

std::string sceneFileName(const char* input) {
    std::string name(input);
    if (name.size() >= 5 && name.substr(name.size() - 5) == ".json")
        name.resize(name.size() - 5);
    if (name.empty() || name == "." || name == ".." ||
        name.find_first_of("<>:\"/\\|?*") != std::string::npos)
        return {};
    return name + ".json";
}
}

void drawSceneFileButton() {
    if (ImGui::Button("场景文件", {100, 34})) ImGui::OpenPopup("保存或打开场景");
}

void drawSceneFileDialog(Scene& scene, Camera& camera, Renderer& renderer,
                         Notifications& notifications) {
    static fs::path directory;
    static fs::path pendingOverwrite;
    static char sceneName[256] = "my-scene";
    static std::string lastError;
    if (!ImGui::BeginPopupModal("保存或打开场景", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) return;
    if (ImGui::IsWindowAppearing()) lastError.clear();

    const auto saveTo = [&](const fs::path& file) {
        const auto result = saveSceneFile(file, scene, camera);
        lastError = result.ok ? "" : "保存失败：" + result.message;
        notifications.set(result.ok ? "场景已保存到：" + file.u8string() : lastError);
        return result.ok;
    };

    ImGui::TextWrapped("选择保存文件夹并填写场景名称，程序会在其中生成场景文件及资源目录。");
    if (ImGui::Button("选择保存文件夹...", {180, 34})) {
        fs::path chosen;
        std::string error;
        const auto picked = pickPath(true, directory, chosen, error);
        if (picked == PickResult::Selected) { directory = std::move(chosen); lastError.clear(); }
        else if (picked == PickResult::Failed) lastError = error;
    }
    ImGui::SameLine();
    if (directory.empty()) ImGui::TextDisabled("尚未选择文件夹");
    else ImGui::TextWrapped("%s", directory.u8string().c_str());

    ImGui::TextUnformatted("场景名称");
    ImGui::SetNextItemWidth(440);
    if (ImGui::InputText("##SceneName", sceneName, sizeof(sceneName))) lastError.clear();
    ImGui::SameLine();
    ImGui::TextDisabled("自动生成场景文件");

    if (ImGui::Button("保存到所选文件夹", {180, 34})) {
        const std::string fileName = sceneFileName(sceneName);
        if (directory.empty()) lastError = "请先选择保存文件夹。";
        else if (fileName.empty()) lastError = "场景名称不能为空，也不能包含路径或特殊字符。";
        else if (!fs::is_directory(directory)) lastError = "所选文件夹已不存在，请重新选择。";
        else {
            const fs::path file = directory / fs::u8path(fileName);
            if (fs::exists(file)) {
                pendingOverwrite = file;
                ImGui::OpenPopup("确认覆盖");
            } else if (saveTo(file)) ImGui::CloseCurrentPopup();
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("打开已有场景...", {160, 34})) {
        fs::path chosen;
        std::string error;
        const auto picked = pickPath(false, directory, chosen, error);
        if (picked == PickResult::Failed) lastError = error;
        if (picked == PickResult::Selected) {
            const auto result = loadSceneFile(chosen, scene, camera);
            lastError = result.ok ? "" : "打开失败：" + result.message;
            notifications.set(result.ok ? "场景已打开：" + chosen.u8string() : lastError);
            if (result.ok) {
                renderer.invalidateSceneResources();
                directory = chosen.parent_path();
                const auto stem = chosen.stem().u8string();
                std::snprintf(sceneName, sizeof(sceneName), "%s", stem.c_str());
                ImGui::CloseCurrentPopup();
            }
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("取消", {90, 34})) ImGui::CloseCurrentPopup();
    if (!lastError.empty()) ImGui::TextWrapped("%s", lastError.c_str());

    bool savedFromOverwrite = false;
    if (ImGui::BeginPopupModal("确认覆盖", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextWrapped("该场景已存在，保存会覆盖原有内容。是否继续？");
        ImGui::TextWrapped("%s", pendingOverwrite.u8string().c_str());
        if (ImGui::Button("覆盖保存", {110, 32})) {
            const fs::path target = pendingOverwrite;
            savedFromOverwrite = saveTo(target);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("返回", {90, 32})) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
    if (savedFromOverwrite) ImGui::CloseCurrentPopup();
    ImGui::EndPopup();
}
}
