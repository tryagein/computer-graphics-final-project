#include "renderer/SceneRenderer.hpp"
#include "assets/ObjImporter.hpp"
#include <glm/gtc/type_ptr.hpp>

namespace scene_editor {
const std::vector<SceneRenderer::Submesh>& SceneRenderer::imported(const std::string& path) {
    auto it = imported_.find(path);
    if (it != imported_.end()) return it->second;
    auto asset = importObj(path);
    std::vector<Submesh> parts;
    if (asset.valid()) {
        for (auto& source : asset.submeshes) {
            Submesh part;
            part.material = source.material;
            part.mesh.upload(source.vertices, source.indices);
            parts.push_back(std::move(part));
        }
    }
    return imported_.emplace(path, std::move(parts)).first->second;
}

void SceneRenderer::draw(const Scene& scene, const ShaderProgram& shader,
                         const Mesh& cube, const Mesh& sphere, const Mesh& plane) {
    for (const auto& object : scene.objects) {
        if (!object.visible) continue;
        const auto model = object.transform.matrix();
        glUniformMatrix4fv(shader.uniform("model"), 1, GL_FALSE, glm::value_ptr(model));
        if (object.kind == ObjectKind::ImportedModel) {
            for (const auto& part : imported(object.geometryPath)) {
                materials_.bind(object.materialOverride ? object.material : part.material, shader);
                part.mesh.draw();
            }
        } else {
            materials_.bind(object.material, shader);
            switch (object.kind) {
            case ObjectKind::Cube: cube.draw(); break;
            case ObjectKind::Sphere: sphere.draw(); break;
            case ObjectKind::Plane: plane.draw(); break;
            case ObjectKind::ImportedModel: break;
            }
        }
    }
}

void SceneRenderer::drawDepth(const Scene& scene, const ShaderProgram& shader,
                              const Mesh& cube, const Mesh& sphere, const Mesh& plane) {
    for (const auto& object : scene.objects) {
        if (!object.visible) continue;
        const auto model = object.transform.matrix();
        glUniformMatrix4fv(shader.uniform("model"), 1, GL_FALSE, glm::value_ptr(model));
        if (object.kind == ObjectKind::ImportedModel) {
            for (const auto& part : imported(object.geometryPath)) part.mesh.draw();
        } else {
            switch (object.kind) {
            case ObjectKind::Cube: cube.draw(); break;
            case ObjectKind::Sphere: sphere.draw(); break;
            case ObjectKind::Plane: plane.draw(); break;
            case ObjectKind::ImportedModel: break;
            }
        }
    }
}
}
