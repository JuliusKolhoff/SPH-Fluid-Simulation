#pragma once
#include <glm/glm.hpp>
using glm::vec3, glm::vec4;
struct SimParams
{
    static constexpr float world_scale = 1.0f;
	const unsigned int n = 400000;

    float particle_radius = 0.16f;
    bool draw_obstacles = false;

    //BOunds
    vec3 bounds = vec3(50, 30, -30);
    float collision_damping = 0.8f;
    //CFL
    float time_scale = 1.0f;
    int   max_substeps = 5;
    float cfl_lamda = 0.4f;

    float step_size = 0.01f;


    //TODO MAke lower  viscoity possible
    /*float target_density = 20.26f;
    float k = 60000.0f; 
    float viscosity = 1.0f;*/

    float smoothing_radius = 0.736f;
    float target_density = 20.76f;
    float k = 24700.0f;
    float viscosity = 6.0f;
    float gravity = 9.81f;

    unsigned int numCells = 0;

    bool run_simulation = false;
};



struct SimParamsGPU {                 // std140, 80 bytes
    float h, h2;
    float target_density, stiffness;
    float viscosity, gravity;
    float collision_damping, particle_radius;
    float poly6_coef, spiky_grad_coef;
    float count;
    float _pad = 0;
    glm::vec4 bounds_min;
    glm::vec4 bounds_max;
    SimParamsGPU(SimParams& params) {
        h = params.smoothing_radius;
        h2 = h * h;
        target_density = params.target_density;
        stiffness = params.k;
        viscosity = params.viscosity;
        gravity = params.gravity;
        collision_damping = params.collision_damping;
        particle_radius = params.particle_radius;
        const float PI = 3.14159265f;
        float r = h;
        float h3 = h2 * r, h6 = h3 * h3, h9 = h6 * h3;
        h = r;
        poly6_coef = 315.0f / (64.0f * PI * h9);
        spiky_grad_coef = -45.0f / (PI * h6);
        count = params.n;
        bounds_min = vec4(glm::min(glm::vec3(0), params.bounds),1);
        bounds_max = vec4(glm::max(glm::vec3(0), params.bounds),1);
    }
};
static_assert(sizeof(SimParamsGPU) == 80);
static_assert(offsetof(SimParamsGPU, bounds_min) == 48);

struct FrameParamsGPU {
    float dt;
    float step_size;
};