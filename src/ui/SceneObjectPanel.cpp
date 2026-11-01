#include "ui/SceneObjectPanel.hpp"
#include <imgui.h>
#include <atomic>
#include "assets/AssetService.hpp"
namespace scene_editor {
void drawSceneObjectPanel(Scene& scene,Notifications& notifications){
    static std::atomic<unsigned> nextId{1};
    ImGui::Begin("Objects");
    for(auto kind:{ObjectKind::Cube,ObjectKind::Sphere,ObjectKind::Plane}){
        const char* label=kind==ObjectKind::Cube?"Add Cube":kind==ObjectKind::Sphere?"Add Sphere":"Add Plane";
        if(ImGui::Button(label)){SceneObject object;object.kind=kind;object.name=label+4;object.id="object-"+std::to_string(nextId++);scene.add(object);}
        ImGui::SameLine();
    }
    ImGui::NewLine();
    static char importPath[512]{};
    ImGui::InputText("OBJ path",importPath,sizeof(importPath));
    static std::string importError;
    if(ImGui::Button("Import OBJ")){if(!addImportedObj(scene,importPath,importError)){notifications.report(Result::failure(importError));ImGui::OpenPopup("Import failed");}else notifications.set("OBJ imported successfully.");}
    if(ImGui::BeginPopupModal("Import failed",nullptr,ImGuiWindowFlags_AlwaysAutoResize)){ImGui::TextWrapped("%s",importError.c_str());if(ImGui::Button("OK"))ImGui::CloseCurrentPopup();ImGui::EndPopup();}
    for(const auto& object:scene.objects){const std::string label=object.name+"##"+object.id;if(ImGui::Selectable(label.c_str(),scene.selectedId==object.id))scene.selectedId=object.id;}
    if(auto* selected=scene.find(scene.selectedId)){
        ImGui::Separator();ImGui::Text("ID: %s",selected->id.c_str());
        ImGui::Checkbox("Visible",&selected->visible);
        ImGui::DragFloat3("Position",&selected->transform.position.x,0.05F);
        ImGui::DragFloat3("Rotation",&selected->transform.rotationDegrees.x,0.5F);
        glm::vec3 proposed=selected->transform.scale;
        if(ImGui::DragFloat3("Scale",&proposed.x,0.02F,0.001F,100.0F))if(proposed.x>0&&proposed.y>0&&proposed.z>0)selected->transform.scale=proposed;
        if(ImGui::Button("Delete Selected"))scene.erase(selected->id);
    }
    ImGui::End();
}
}
