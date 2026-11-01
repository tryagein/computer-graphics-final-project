#pragma once
#include "renderer/ShaderProgram.hpp"
#include "renderer/Texture.hpp"
#include "scene/Scene.hpp"
#include <memory>
#include <string>
#include <unordered_map>
namespace scene_editor {class MaterialBinding{public:void bind(const Material&,const ShaderProgram&);void clear(){textures_.clear();}private:std::unordered_map<std::string,std::unique_ptr<Texture>> textures_;};}
