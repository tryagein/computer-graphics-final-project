#pragma once
#include <glad/gl.h>
#include <string>
namespace scene_editor {class Texture {public:GLuint id{};Texture()=default;Texture(const Texture&)=delete;Texture&operator=(const Texture&)=delete;Texture(Texture&& other) noexcept:id(other.id){other.id=0;}Texture&operator=(Texture&& other)noexcept{if(this!=&other){reset();id=other.id;other.id=0;}return*this;}~Texture(){reset();}bool load(const std::string& path);void reset(){if(id){glDeleteTextures(1,&id);id=0;}}};}
