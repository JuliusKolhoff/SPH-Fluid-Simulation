#pragma once
template <typename T>
class UniformBuffer {
public:
    explicit UniformBuffer(GLuint binding) {
        glGenBuffers(1, &id);
        glBindBuffer(GL_UNIFORM_BUFFER, id);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(T), nullptr, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_UNIFORM_BUFFER, binding, id);   // binds once, stays bound
    }
    void Upload(const T& v) const {
        glBindBuffer(GL_UNIFORM_BUFFER, id);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(T), &v);
    }
    ~UniformBuffer() { glDeleteBuffers(1, &id); }
    UniformBuffer(const UniformBuffer&) = delete;
    UniformBuffer& operator=(const UniformBuffer&) = delete;
    GLuint id = 0;
};