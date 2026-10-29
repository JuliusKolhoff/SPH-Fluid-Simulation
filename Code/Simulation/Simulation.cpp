#include "Simulation.h"

Simulation* Simulation::instance = nullptr;

float Simulation::step(float dt, FrameParamsGPU& fd, UniformBuffer<FrameParamsGPU>& ubo) {

    fd.dt = dt; fd.step_size = dt; ubo.Upload(fd);

    clear_grid.Dispatch(params.numCells);
    ComputeShader::Barrier();
    count_cells.Dispatch(params.n);
    ComputeShader::Barrier();
    scan.Dispatch(1);
    ComputeShader::Barrier();
    scatter.Dispatch(params.n);
    ComputeShader::Barrier();
    compDensity.Dispatch(params.n);
    ComputeShader::Barrier();
    forces.Dispatch(params.n);
    ComputeShader::Barrier();
    integrate.Dispatch(params.n);
    ComputeShader::Barrier();
    glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
    return dt;
}

    float Simulation::gpu_cfl() {
        const GLuint zero[2] = { 0, 0 };
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, maxBuf.id);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(zero), zero);
        reduceMax.Dispatch(params.n);
        ComputeShader::Barrier();

        GLuint bits[2];
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(bits), bits);
        float v2, a2; memcpy(&v2, &bits[0], 4); memcpy(&a2, &bits[1], 4);

        float h = params.smoothing_radius;
        float c = sqrtf(7.0f * params.k / params.target_density);
        float vmax = std::max(1.0f, sqrtf(v2));
        float amax = sqrtf(a2);

        float dt = params.cfl_lamda * h / (c + vmax);
        if (amax > 1e-6f) dt = std::min(dt, 0.25f * sqrtf(h / amax));
        return dt;
    }

    void Simulation::advance(float frame_dt, FrameParamsGPU & fd, UniformBuffer<FrameParamsGPU>&ubo) {
        float remaining = std::min(frame_dt, 0.05f) * params.time_scale;
        float dt = gpu_cfl();

        int n = std::min(params.max_substeps, (int)std::ceil(remaining / dt));
        float sdt = (n * dt >= remaining) ? remaining / n : dt;  // hit frame time exactly, or run at dt if capped

        for (int i = 0; i < n; ++i) step(sdt, fd, ubo);
    }

