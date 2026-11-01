#include "renderer/ShaderProgram.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
namespace scene_editor {
static std::string readText(const std::string&p){std::ifstream f(p);std::stringstream s;s<<f.rdbuf();return s.str();}
GLuint ShaderProgram::compile(GLenum t,const std::string&s){GLuint h=glCreateShader(t);const char*p=s.c_str();glShaderSource(h,1,&p,nullptr);glCompileShader(h);GLint ok=0;glGetShaderiv(h,GL_COMPILE_STATUS,&ok);if(!ok){char log[2048]{};glGetShaderInfoLog(h,sizeof(log),nullptr,log);std::cerr<<log<<'\n';glDeleteShader(h);return 0;}return h;}
bool ShaderProgram::load(const std::string&v,const std::string&f){const auto vs=readText(v),fs=readText(f);if(vs.empty()||fs.empty()){std::cerr<<"Shader source missing: "<<v<<" or "<<f<<'\n';return false;}GLuint a=compile(GL_VERTEX_SHADER,vs);if(!a)return false;GLuint b=compile(GL_FRAGMENT_SHADER,fs);if(!b){glDeleteShader(a);return false;}GLuint p=glCreateProgram();glAttachShader(p,a);glAttachShader(p,b);glLinkProgram(p);glDeleteShader(a);glDeleteShader(b);GLint ok=0;glGetProgramiv(p,GL_LINK_STATUS,&ok);if(!ok){char log[2048]{};glGetProgramInfoLog(p,sizeof(log),nullptr,log);std::cerr<<log<<'\n';glDeleteProgram(p);return false;}reset();id=p;return true;}
ShaderProgram::~ShaderProgram(){reset();}
}
