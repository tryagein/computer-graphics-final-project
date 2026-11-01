#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "renderer/Texture.hpp"
namespace scene_editor {bool Texture::load(const std::string& path){int w=0,h=0,n=0;stbi_set_flip_vertically_on_load(true);unsigned char* data=stbi_load(path.c_str(),&w,&h,&n,4);if(!data)return false;reset();glGenTextures(1,&id);glBindTexture(GL_TEXTURE_2D,id);glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,w,h,0,GL_RGBA,GL_UNSIGNED_BYTE,data);glGenerateMipmap(GL_TEXTURE_2D);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);stbi_image_free(data);return true;}}
