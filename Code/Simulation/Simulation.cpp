#include "Simulation.h"

Simulation* Simulation::instance = nullptr;

void Simulation::step(float dt) {
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
}

void Simulation::calculate_cfl(float max_dt) {
    float v_max = 1.0f;   // avoid div/0
    #pragma omp parallel for schedule(dynamic, 32)
    for (int i = 0; i < params.n; ++i)
        v_max = glm::max(v_max, length(vec3(vel[i])));

    float c = sqrtf(7.0f * params.k / params.target_density);   // speed of sound
    params.step_size = params.cfl_lamda * params.smoothing_radius / (v_max + c);
    params.step_size = glm::min(params.step_size, max_dt);
}