#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Compute/ComputeShader.h"
#include "SimParams.h"
#include "Compute/Ubo.h"
#include "Compute/GpuBuffer.h"

class Simulation {
    static Simulation* instance;
    ComputeShader clear_grid =  ComputeShader("clear_grid.comp"),
        count_cells = ComputeShader("count_cells.comp"),
        scan = ComputeShader("scan.comp"),
        scatter = ComputeShader("scatter.comp"),
        reduceMax = ComputeShader("reduce_max.comp"),
        compDensity = ComputeShader("density.comp"),
        forces = ComputeShader("forces.comp"),
        integrate = ComputeShader("integrate.comp");
        //cs = ComputeShader("simulation.comp");
        ComputeShader cs = ComputeShader("simulation.comp");
        GpuBuffer maxBuf = GpuBuffer(2 * sizeof(GLuint), nullptr, GL_DYNAMIC_READ);
    Simulation(){
        pos = new glm::vec4[params.n]();
        color = new glm::vec4[params.n]();
        vel = new glm::vec4[params.n]();
        accel = new glm::vec4[params.n]();
        mass = new float[params.n];
        density = new glm::vec2[params.n];
        pressure = new float[params.n]();
        movable = new unsigned int[params.n];

        std::fill(mass, mass + params.n, 1.0f);
        std::fill(density, density + params.n, glm::vec2(1.0f));
        std::fill(movable, movable + params.n, 1u);

        auto& sim = *this;

        const glm::vec3 bmin = glm::min(glm::vec3(0), params.bounds);
        const glm::vec3 bmax = glm::max(glm::vec3(0), params.bounds);
        const glm::vec3 size = bmax - bmin;

        const int side = (int)std::ceil(std::cbrt((float)params.n));   // 22 for 10000 (22^3 = 10648)

        // Lattice spacing, shrunk if needed so the cube fits inside the bounds
        float spacing = params.smoothing_radius * 0.5f;
        float maxExtent = glm::min(size.x, glm::min(size.y, size.z)) - 2.0f * params.particle_radius;
        spacing = glm::min(spacing, maxExtent / (side - 1));

        const glm::vec3 extent = glm::vec3(spacing * (side - 1));
        const glm::vec3 start = (bmin + bmax) * 0.5f - extent * 0.5f;   // centred in the box

        auto rnd = []() { return (float)rand() / RAND_MAX - 0.5f; };      // -0.5 .. 0.5
        const float jitter = 0.05f * spacing;                             // breaks perfect symmetry

        for (unsigned int i = 0; i < params.n; i++) {
            int x = i % side;
            int y = (i / side) % side;
            int z = i / (side * side);

            glm::vec3 p = start + glm::vec3(x, y, z) * spacing
                + glm::vec3(rnd(), rnd(), rnd()) * jitter;

            sim.pos[i] = glm::vec4(p, 1.0f);
            sim.vel[i] = glm::vec4(0.0f);
            sim.color[i] = glm::vec4(1.0f);
        }
    }
    ~Simulation() {
        delete[] pos;
        delete[] color;
        delete[] vel;
        delete[] mass;
        delete[] density;
        delete[] pressure;
        delete[] movable;
    }
public:

    glm::vec4 *pos;
    glm::vec4 *color;
    glm::vec4 *vel;
    glm::vec4 *accel;
    float *mass;
    glm::vec2 *density; //  normal/near
    float *pressure;
    unsigned int *movable;

    struct SimParams params;
    static Simulation& get_instance() {
        if (instance == nullptr) {
            instance = new Simulation();
        }
        return *instance;
    }

	float step(float dt, FrameParamsGPU& fd, UniformBuffer<FrameParamsGPU>& ubo);
    void advance(float frame_dt, FrameParamsGPU& fd, UniformBuffer<FrameParamsGPU>& ubo);
    float gpu_cfl();
};