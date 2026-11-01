#pragma once
#include "renderer/MaterialBinding.hpp"
#include "renderer/Mesh.hpp"
#include "scene/Scene.hpp"
#include <string>
#include <unordered_map>
#include <vector>
namespace scene_editor {
class SceneRenderer {
public:
    void draw(const Scene&, const ShaderProgram&, const Mesh&, const Mesh&, const Mesh&);
    void drawDepth(const Scene&, const ShaderProgram&, const Mesh&, const Mesh&, const Mesh&);
    void shutdown() { imported_.clear(); materials_.clear(); }
private:
    struct Submesh { Mesh mesh; Material material; };
    const std::vector<Submesh>& imported(const std::string& path);
    std::unordered_map<std::string, std::vector<Submesh>> imported_;
    MaterialBinding materials_;
};
}
