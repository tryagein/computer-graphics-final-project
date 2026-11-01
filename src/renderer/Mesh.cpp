#include "renderer/Mesh.hpp"
#include <cstddef>
namespace scene_editor {
Mesh::Mesh(Mesh&&o)noexcept:vao(o.vao),vbo(o.vbo),ebo(o.ebo),indexCount(o.indexCount){o.vao=o.vbo=o.ebo=0;o.indexCount=0;}
Mesh&Mesh::operator=(Mesh&&o)noexcept{if(this!=&o){if(ebo)glDeleteBuffers(1,&ebo);if(vbo)glDeleteBuffers(1,&vbo);if(vao)glDeleteVertexArrays(1,&vao);vao=o.vao;vbo=o.vbo;ebo=o.ebo;indexCount=o.indexCount;o.vao=o.vbo=o.ebo=0;o.indexCount=0;}return*this;}
void Mesh::reset(){if(ebo)glDeleteBuffers(1,&ebo);if(vbo)glDeleteBuffers(1,&vbo);if(vao)glDeleteVertexArrays(1,&vao);vao=vbo=ebo=0;indexCount=0;}
Mesh::~Mesh(){reset();}
void Mesh::upload(const std::vector<Vertex>&v,const std::vector<unsigned int>&i){if(!vao)glGenVertexArrays(1,&vao);if(!vbo)glGenBuffers(1,&vbo);if(!ebo)glGenBuffers(1,&ebo);glBindVertexArray(vao);glBindBuffer(GL_ARRAY_BUFFER,vbo);glBufferData(GL_ARRAY_BUFFER,GLsizeiptr(v.size()*sizeof(Vertex)),v.data(),GL_STATIC_DRAW);glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,ebo);glBufferData(GL_ELEMENT_ARRAY_BUFFER,GLsizeiptr(i.size()*sizeof(unsigned)),i.data(),GL_STATIC_DRAW);glEnableVertexAttribArray(0);glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)offsetof(Vertex,position));glEnableVertexAttribArray(1);glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)offsetof(Vertex,normal));glEnableVertexAttribArray(2);glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)offsetof(Vertex,uv));indexCount=GLsizei(i.size());glBindVertexArray(0);}
void Mesh::draw()const{glBindVertexArray(vao);glDrawElements(GL_TRIANGLES,indexCount,GL_UNSIGNED_INT,nullptr);glBindVertexArray(0);}
}
