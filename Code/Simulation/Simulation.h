#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Compute/ComputeShader.h"
#include "SimParams.h"

class Simulation {
    static Simulation* instance;
    ComputeShader clear_grid = ComputeShader("clear_grid.comp"),
        count_cells = ComputeShader("count_cells.comp"),
    scan = ComputeShader("scan.comp"),
    scatter = ComputeShader("scatter.comp");
        /*density = ComputeShader(""),
        forces = ComputeShader(""),
        integrate = ComputeShader(""),
        cs = ComputeShader("simulation.comp");*/
    ComputeShader cs = ComputeShader("simulation.comp");
    Simulation(){
        pos = new glm::vec4[params.n];
        color = new glm::vec4[params.n];
        vel = new glm::vec4[params.n];
        mass = new float[params.n]{1.0f};
        density = new glm::vec2[params.n]{ glm::vec2 (1.0f,1.0f)};
        pressure = new float[params.n] {0.0f};
        movable = new unsigned int[params.n] {true};
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

	void step(float dt);

};