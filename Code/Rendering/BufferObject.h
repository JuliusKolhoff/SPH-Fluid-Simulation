#ifndef BufferObject_H
#define BufferObject_H

#include <glad/glad.h>
#include <cstddef> // for std::size_t
#include <iterator> // for std::begin and std::end

class BufferObject
{
private:
    GLenum target;
public:
    GLuint ID;
    

    template<typename Container>
    BufferObject(GLenum target, const Container& data) : target(target) {
        glGenBuffers(1, &ID);

        glBindBuffer(target, ID);
        glBufferData(target, data.size() * sizeof(typename Container::value_type), data.data(), GL_STATIC_DRAW);
    }

    void Bind() const {
        glBindBuffer(target, ID);
    }

    void Unbind() const {
        glBindBuffer(target, 0);
    }

    ~BufferObject() {
        glDeleteBuffers(1, &ID);
    }
};
#endif