#include "ui/SceneObjectPanel.hpp"
#include <imgui.h>
#include <string>

namespace scene_editor {
namespace {
unsigned nextObjectId = 1;
const char* kindName(ObjectKind kind) {
    switch (kind) {
    case ObjectKind::Cube: return "立方体";
    case ObjectKind::Sphere: return "球体";
    case ObjectKind::Plane: return "平面";
    case ObjectKind::ImportedModel: return "模型";
    }
    return "对象";
}
}

bool addPrimitive(Scene& scene, ObjectKind kind, Notifications& notifications) {
    SceneObject object;
    object.kind = kind;
    object.name = std::string(kindName(kind)) + " " + std::to_string(nextObjectId);
    do { object.id = "object-" + std::to_string(nextObjectId++); }
    while (scene.find(object.id));
    const unsigned slot = (nextObjectId - 2) % 4;
    object.transform.position = {
        slot == 0 ? 1.6F : slot == 1 ? -1.6F : 0.0F,
        kind == ObjectKind::Plane ? 0.02F : 0.5F,
        slot == 2 ? 1.6F : slot == 3 ? -1.6F : 0.0F
    };
    if (kind == ObjectKind::Plane) object.transform.scale = {1.5F, 1.0F, 1.5F};
    if (!scene.add(object)) {
        notifications.report(Result::failure("无法创建对象。"));
        return false;
    }
    scene.selectedId = object.id;
    notifications.set("已创建 " + object.name + "，可在右侧编辑属性。");
    return true;
}

void drawSceneObjectPanel(Scene& scene) {
    ImGui::TextDisabled("场景对象");
    ImGui::Separator();
    if (scene.objects.empty()) ImGui::TextWrapped("场景为空，请使用顶部工具栏添加对象。");
    for (auto& object : scene.objects) {
        ImGui::PushID(object.id.c_str());
        ImGui::Checkbox("##visible", &object.visible);
        ImGui::SameLine();
        const std::string label = std::string(kindName(object.kind)) + "  " + object.name;
        if (ImGui::Selectable(label.c_str(), scene.selectedId == object.id)) scene.selectedId = object.id;
        ImGui::PopID();
    }
    ImGui::Spacing();
    ImGui::TextDisabled("勾选框控制对象可见性");
}

void drawObjectInspector(Scene& scene) {
    auto* selected = scene.find(scene.selectedId);
    if (!selected) {
        ImGui::TextWrapped("从左侧选择对象，或从顶部工具栏添加对象。");
        return;
    }
    ImGui::Text("%s", selected->name.c_str());
    ImGui::TextDisabled("%s", selected->id.c_str());
    ImGui::SeparatorText("变换");
    ImGui::DragFloat3("位置", &selected->transform.position.x, 0.05F);
    ImGui::DragFloat3("旋转", &selected->transform.rotationDegrees.x, 0.5F);
    glm::vec3 proposed = selected->transform.scale;
    if (ImGui::DragFloat3("缩放", &proposed.x, 0.02F, 0.001F, 100.0F) &&
        proposed.x > 0 && proposed.y > 0 && proposed.z > 0) selected->transform.scale = proposed;
    ImGui::Checkbox("可见", &selected->visible);
    ImGui::SameLine();
    if (ImGui::Button("删除对象")) scene.erase(selected->id);
}
}
