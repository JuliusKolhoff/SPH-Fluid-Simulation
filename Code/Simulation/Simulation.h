#pragma once
#define _USE_MATH_DEFINES
#include <vector>
#include <cmath>
#include <random>
#include "../Rendering/Rendering.h"
#include "Particle.h"
#include "Kernels.h"
#include <omp.h>
#include <functional>
#include "GLFW/glfw3.h"

using namespace glm;
static const int hashK1 = 4000009;
static const int hashK2 = 8000011;
static const int hashK3 = 12000017;

class Simulation
{
public:
    static constexpr float PIXELS_PER_METER = 150;

    float particle_radius = 0.16f;
    unsigned int simulation_size = 3000;
    unsigned int obstacle_particle_count = 20000;
    bool draw_obstacles = false;


    //CFL
    float time_scale = 1.0f;   // >1 = fast forward
    int   max_substeps = 1;     // safety cap so a frame can't take forever

    float step_size = 0.01f;

    float smoothing_radius = 0.576f;
    float target_density = 30.0f;
    float k = 27423.0f;
    float viscosity = 6.91f;
    float gravity = 30.81f;
    
    bool right_mouse_down = false;
    vec3 force_point = vec3(0);

    //Crude Rendering Improvement
    bool render_complex = false;
    float radius_increase = 4.0f;
    //float radius_increase = 10.0f;

    //Manager
    KernelManager kernel_manager;

    //Spatial Hash
    Particle *particles = nullptr;
    std::pair<int, int> *spatial_lookup = nullptr;
    int *start_indices = nullptr;

private:
    //debug
    float error_sum = 0;
    float density_sum = 0;
    vec3 sample_point = vec3(0);

    vec3 base_particle_color = { 0.0f,0.0f,1.0f };
    vec3 highlighted_particle_color = { 0.0f,1.0f,0.0f };
    vec3 obstacle_color = { 1.0f,0.0f,0.0f };

    static Simulation* instance;

    Simulation(){
        particles = new Particle[simulation_size + obstacle_particle_count];
        spatial_lookup = new std::pair<int, int>[simulation_size + obstacle_particle_count];
        start_indices = new int[simulation_size + obstacle_particle_count];
    };
    glm::ivec3 cell_offsets[27] =
    {
        // z = -1
        {-1, 1,-1}, {0, 1,-1}, {1, 1,-1},
        {-1, 0,-1}, {0, 0,-1}, {1, 0,-1},
        {-1,-1,-1}, {0,-1,-1}, {1,-1,-1},
        // z = 0
        {-1, 1, 0}, {0, 1, 0}, {1, 1, 0},
        {-1, 0, 0}, {0, 0, 0}, {1, 0, 0},
        {-1,-1, 0}, {0,-1, 0}, {1,-1, 0},
        // z = +1
        {-1, 1, 1}, {0, 1, 1}, {1, 1, 1},
        {-1, 0, 1}, {0, 0, 1}, {1, 0, 1},
        {-1,-1, 1}, {0,-1, 1}, {1,-1, 1}
    };
public:
    static Simulation& get_instance() {
        if (instance == nullptr) {
            instance = new Simulation();
        }
        return *instance;
    }
    ~Simulation() {
        delete[] particles;
        delete[] spatial_lookup;
        delete[] start_indices;
    }
    void create_particles();
    void random_particles();
    float step(float max_dt, GLFWwindow* window);
    void advance(float frame_dt, GLFWwindow* window);
    float calculate_density(Particle &particle);
    float sample_density_contribution(Particle& particle_j);
    float calculate_density_contribution(Particle& particle_i, Particle& particle_j);
    void sample_at_point(vec3 pos);
    void update_vertex_data(std::vector<GLfloat> *vertices);
    vec3 calculate_pressure_gradient(Particle &particle);
    vec3 calculate_pressure_contribution(Particle& particle_i, Particle& particle_j);
    float calculate_pressure(Particle& particle);
    vec3 calculate_pressue_acceleration(Particle& particle);
    vec3 calculate_viscosity_force(Particle& particle_i);
    vec3 calculate_viscosity_contribution(Particle& particle_i, Particle& particle_j);
    void handle_collisions(Particle& particle);
    glm::ivec3 get_cell_coord(vec3 pos);
    int hash_cell(glm::ivec3 cell);
    int KeyFromHash(int hash, int tableSize);
    void update_spacial_lookup();
    template <typename R> R do_for_each_neighbour(Particle& particle_i, R(Simulation::*func)(Particle&,Particle&));
    void print_debug_info();
private:
    void create_obstacles();
   
};
