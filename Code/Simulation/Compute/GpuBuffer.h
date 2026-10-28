#ifndef GPU_BUFFER_H
#define GPU_BUFFER_H

#include <glad/glad.h>
#include <cstddef>
#include <vector>
#include <utility>
#include <iostream>

template<typename T>
class GpuBuffer
{
public:
    GLuint ID = 0;

    GpuBuffer() = default;

    // allocate `count` elements, uninitialised
    GpuBuffer(std::size_t count, GLenum usage = GL_DYNAMIC_DRAW)
        : count_(count), usage_(usage)
    {
        create();
        allocate(nullptr, count_);
    }

    // allocate and upload data
    GpuBuffer(const T* data, std::size_t count, GLenum usage = GL_DYNAMIC_DRAW)
        : count_(count), usage_(usage)
    {
        create();
        allocate(data, count_);
    }

    GpuBuffer(const std::vector<T>& data, GLenum usage = GL_DYNAMIC_DRAW)
        : GpuBuffer(data.data(), data.size(), usage) {
    }

    GpuBuffer(const GpuBuffer&) = delete;
    GpuBuffer& operator=(const GpuBuffer&) = delete;

    GpuBuffer(GpuBuffer&& o)
        : ID(o.ID), count_(o.count_), usage_(o.usage_)
    {
        o.ID = 0;
        o.count_ = 0;
    }
    GpuBuffer& operator=(GpuBuffer&& o)
    {
        if (this != &o) {
            release();
            ID = o.ID; count_ = o.count_; usage_ = o.usage_;
            o.ID = 0;  o.count_ = 0;
        }
        return *this;
    }

    ~GpuBuffer() { release(); }

    void BindBase(GLuint binding) const
    {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, ID);
    }
    void BindRange(GLuint binding, std::size_t firstElement, std::size_t elementCount) const
    {
        glBindBufferRange(GL_SHADER_STORAGE_BUFFER, binding, ID,
            (GLintptr)(firstElement * sizeof(T)),
            (GLsizeiptr)(elementCount * sizeof(T)));
    }
    void Bind() const { glBindBuffer(GL_SHADER_STORAGE_BUFFER, ID); }
    void Unbind() const { glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); }

    void Upload(const T* data, std::size_t n, std::size_t offset = 0)
    {
        if (offset + n > count_) {
            std::cout << "ERROR::GPUBUFFER::UPLOAD_OUT_OF_RANGE" << std::endl;
            return;
        }
        Bind();
        glBufferSubData(GL_SHADER_STORAGE_BUFFER,
            (GLintptr)(offset * sizeof(T)),
            (GLsizeiptr)(n * sizeof(T)), data);
        Unbind();
    }
    void Upload(const std::vector<T>& data, std::size_t offset = 0)
    {
        Upload(data.data(), data.size(), offset);
    }

    void Download(T* out, std::size_t n, std::size_t offset = 0) const
    {
        if (offset + n > count_) {
            std::cout << "ERROR::GPUBUFFER::DOWNLOAD_OUT_OF_RANGE" << std::endl;
            return;
        }
        Bind();
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER,
            (GLintptr)(offset * sizeof(T)),
            (GLsizeiptr)(n * sizeof(T)), out);
        Unbind();
    }
    std::vector<T> Download() const
    {
        std::vector<T> out(count_);
        if (count_ > 0) Download(out.data(), count_);
        return out;
    }

    void Resize(std::size_t newCount, const T* data = nullptr)
    {
        if (ID == 0) create();
        count_ = newCount;
        allocate(data, count_);
    }

    void Clear()
    {
        std::vector<T> zeros(count_, T{});
        if (count_ > 0) Upload(zeros.data(), count_);
    }

    // ---- info -------------------------------------------------------------
    std::size_t Count() const { return count_; }
    std::size_t Bytes() const { return count_ * sizeof(T); }
    bool        Valid() const { return ID != 0; }

private:
    std::size_t count_ = 0;
    GLenum      usage_ = GL_DYNAMIC_DRAW;

    void create() { glGenBuffers(1, &ID); }

    void allocate(const T* data, std::size_t n)
    {
        Bind();
        glBufferData(GL_SHADER_STORAGE_BUFFER, (GLsizeiptr)(n * sizeof(T)), data, usage_);
        Unbind();
    }

    void release()
    {
        if (ID != 0) {
            glDeleteBuffers(1, &ID);
            ID = 0;
        }
    }
};

#endif