#pragma once
#include "renderer/Mesh.hpp"
#include "scene/Scene.hpp"
#include <string>
#include <vector>
namespace scene_editor {struct ImportedSubmesh{std::vector<Vertex> vertices;std::vector<unsigned> indices;Material material;};struct ImportedAsset{std::vector<ImportedSubmesh> submeshes;std::string error;bool valid()const{return error.empty()&&!submeshes.empty();}};ImportedAsset importObj(const std::string& path);}
