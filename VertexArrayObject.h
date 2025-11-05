#ifndef VertexArrayObject_H
#define VertexArrayObject_H

#include <glad/glad.h>
#include <cstddef> // for std::size_t
#include <iterator> // for std::begin and std::end

class VertexArrayObject
{
public:
    GLuint ID;

    VertexArrayObject(){
        glGenVertexArrays(1, &ID);
        glBindVertexArray(ID);
    }

    void Bind() const {
        glBindVertexArray(ID);
    }

    void Unbind() const {
        glBindVertexArray(0);
    }

    ~VertexArrayObject() {
        glDeleteVertexArrays(1, &ID);
    }
};
#endif