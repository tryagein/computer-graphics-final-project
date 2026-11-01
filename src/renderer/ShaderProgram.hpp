#pragma once
#include <glad/gl.h>
#include <string>
namespace scene_editor { class ShaderProgram { public: GLuint id{}; bool load(const std::string&,const std::string&);void use()const{glUseProgram(id);}GLint uniform(const char*n)const{return glGetUniformLocation(id,n);}void reset(){if(id){glDeleteProgram(id);id=0;}}~ShaderProgram();private:static GLuint compile(GLenum,const std::string&);}; }
