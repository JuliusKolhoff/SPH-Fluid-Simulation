#pragma once
#include <glm/glm.hpp>
using glm::vec3, glm::vec4;
struct SimParams
{
    static constexpr float world_scale = 1.0f;
	const unsigned int n = 1000;

    float particle_radius = 0.16f;
    bool draw_obstacles = false;

    //BOunds
    vec3 bounds = vec3(10, 10, -10);

    //CFL
    float time_scale = 1.0f;
    int   max_substeps = 5;
    float cfl_lamda = 0.6f;

    float step_size = 0.01f;

    float smoothing_radius = 0.736f;
    float target_density = 30.76f;
    float k = 24700.0f;
    float viscosity = 6.0f;
    float gravity = 9.81f;

    const unsigned int numCells = 0;
};