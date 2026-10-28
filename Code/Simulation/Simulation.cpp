#include "Simulation.h"

Simulation* Simulation::instance = nullptr;

void Simulation::step(float dt) {
    /*cs.use();
    cs.setFloat("dt", dt);
    cs.setVec3("gravity", 0, -9.8f, 0);
    cs.setUInt("count", params.n);
    cs.Dispatch(params.n);*/
    clear_grid.Dispatch(params.numCells);   
    ComputeShader::Barrier();
    count_cells.Dispatch(params.n);         
    ComputeShader::Barrier();
    scan.Dispatch(1);
    ComputeShader::Barrier();
    scatter.Dispatch(params.n);
    ComputeShader::Barrier();
    /*density.Dispatch(params.N);
    ComputeShader::Barrier();
    forces.Dispatch(params.N);
    ComputeShader::Barrier();
    integrate.Dispatch(params.N);
    ComputeShader::Barrier();*/
}