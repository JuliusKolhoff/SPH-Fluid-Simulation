#include "Simulation.h"

Simulation* Simulation::instance = nullptr;

float Simulation::step(float dt,FrameParamsGPU &frameData, UniformBuffer<FrameParamsGPU>& frameUbo) {
    frameData.dt = dt;
    frameData.step_size = calculate_cfl(dt);
    frameUbo.Upload(frameData);
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
    return frameData.step_size;
}

void Simulation::advance(float frame_dt,FrameParamsGPU& frameData,UniformBuffer<FrameParamsGPU>& frameUbo) {
    float remaining = std::min(frame_dt, 0.05f) * params.time_scale;
    for (int n = 0; n < params.max_substeps && remaining > 1e-5f; ++n)
        remaining -= step(remaining,frameData,frameUbo);
}


float Simulation::calculate_cfl(float max_dt) {
    float v_max = 1.0f;   // avoid div/0
    #pragma omp parallel for schedule(dynamic, 32)
    for (int i = 0; i < params.n; ++i)
        v_max = glm::max(v_max, length(vec3(vel[i])));

    float c = sqrtf(7.0f * params.k / params.target_density);   // speed of sound
    params.step_size = params.cfl_lamda * params.smoothing_radius / (v_max + c);
    params.step_size = glm::min(params.step_size, max_dt);
    return params.step_size;
}