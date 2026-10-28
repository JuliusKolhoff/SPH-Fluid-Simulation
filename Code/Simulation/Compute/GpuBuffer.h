class GpuBuffer {
public:
    GLuint id;
    GpuBuffer(size_t bytes, const void* init = nullptr, GLenum usage = GL_DYNAMIC_COPY) {
        glGenBuffers(1, &id);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
        glBufferData(GL_SHADER_STORAGE_BUFFER, bytes, init, usage);
    }
    void BindBase(GLuint binding) const { glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, id); }
    ~GpuBuffer() { glDeleteBuffers(1, &id); }
};