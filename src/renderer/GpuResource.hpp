#pragma once
#include <glad/gl.h>
namespace scene_editor {
struct GpuResource {
    GLuint id{};
    GpuResource()=default;
    GpuResource(const GpuResource&)=delete;
    GpuResource& operator=(const GpuResource&)=delete;
    GpuResource(GpuResource&& other) noexcept : id(other.id){other.id=0;}
    GpuResource& operator=(GpuResource&& other) noexcept {if(this!=&other){reset();id=other.id;other.id=0;}return *this;}
    virtual ~GpuResource()=default;
    virtual void reset()=0;
};
}
