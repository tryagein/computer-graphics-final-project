#include "ui/LightPanel.hpp"
#include <imgui.h>
#include <algorithm>
#include <cstddef>
#include <string>

namespace scene_editor {
void drawLightPanel(Scene& scene) {
    static unsigned nextId = 1;
    const auto pointCount = std::count_if(scene.lights.begin(), scene.lights.end(),
        [](const Light& light) { return light.type == LightType::Point; });
    const auto directionalCount = std::count_if(scene.lights.begin(), scene.lights.end(),
        [](const Light& light) { return light.type == LightType::Directional; });
    ImGui::TextWrapped("点光源照亮附近物体；平行光照亮整个场景。");
    ImGui::BeginDisabled(pointCount >= 4);
    if (ImGui::Button("添加点光源")) {
        Light light;
        do { light.id = "point-ui-" + std::to_string(nextId++); }
        while (std::any_of(scene.lights.begin(), scene.lights.end(),
            [&](const Light& existing) { return existing.id == light.id; }));
        scene.lights.push_back(light);
    }
    ImGui::EndDisabled();
    ImGui::BeginDisabled(directionalCount >= 2);
    if (ImGui::Button("添加平行光")) {
        Light light;
        do { light.id = "directional-ui-" + std::to_string(nextId++); }
        while (std::any_of(scene.lights.begin(), scene.lights.end(),
            [&](const Light& existing) { return existing.id == light.id; }));
        light.type = LightType::Directional;
        scene.lights.push_back(light);
    }
    ImGui::EndDisabled();
    ImGui::TextDisabled("最多显示 4 个点光源和 2 个平行光。");
    ImGui::SeparatorText("场景灯光");
    for (size_t index = 0; index < scene.lights.size(); ++index) {
        auto& light = scene.lights[index];
        const std::string title = std::string(light.type == LightType::Point ? "点光源 " : "平行光 ") + light.id;
        ImGui::PushID(light.id.c_str());
        if (ImGui::TreeNode(title.c_str())) {
            ImGui::Checkbox("启用", &light.enabled);
            ImGui::ColorEdit3("颜色", &light.color.x);
            ImGui::DragFloat("强度", &light.intensity, 0.01F, 0.0F, 10.0F);
            if (light.type == LightType::Point)
                ImGui::DragFloat3("位置", &light.position.x, 0.05F);
            else
                ImGui::DragFloat3("方向", &light.direction.x, 0.02F);
            if (ImGui::Button("删除灯光")) {
                scene.lights.erase(scene.lights.begin() + static_cast<std::ptrdiff_t>(index));
                ImGui::TreePop();
                ImGui::PopID();
                break;
            }
            ImGui::TreePop();
        }
        ImGui::PopID();
    }
}
}
