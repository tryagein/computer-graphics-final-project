#pragma once
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <vector>
namespace scene_editor { struct Vertex{glm::vec3 position,normal;glm::vec2 uv;}; class Mesh{public:GLuint vao{},vbo{},ebo{};GLsizei indexCount{};Mesh()=default;Mesh(const Mesh&)=delete;Mesh&operator=(const Mesh&)=delete;Mesh(Mesh&&)noexcept;Mesh&operator=(Mesh&&)noexcept;~Mesh();void reset();void upload(const std::vector<Vertex>&,const std::vector<unsigned int>&);void draw()const;}; }
