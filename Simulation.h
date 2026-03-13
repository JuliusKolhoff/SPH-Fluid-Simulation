#pragma once
#define _USE_MATH_DEFINES
#include <vector>
#include <cmath>
#include <random>
#include "Rendering.h"
#include "Particle.h"
#include "Kernels.h"

#include <functional>

using namespace glm;
//hash 1 has 0ne added 5 at the end
static const int hashK1 = 158235;
static const int hashK2 = 9737333;

class Simulation
{
public:
    const float particle_radius = 16.0f;
    const float smoothing_radius = 0.5f;
    unsigned int simulation_size = 500;
    float target_density = 8.9f;
    float step_size = 0.01f;
    float k = 40.0f;
    float viscosity = 15;
    float gravity = 60.0f;

    

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

    static Simulation* instance;

    Simulation(){
        particles = new Particle[simulation_size];
        spatial_lookup = new std::pair<int, int>[simulation_size];
        start_indices = new int[simulation_size];
    };
    glm::vec<2, int> cell_offsets[9] =
    {
        glm::vec<2,int>(-1, 1),
        glm::vec<2,int>(0, 1),
        glm::vec<2,int>(1, 1),
        glm::vec<2,int>(-1, 0),
        glm::vec<2,int>(0, 0),
        glm::vec<2,int>(1, 0),
        glm::vec<2,int>(-1, -1),
        glm::vec<2,int>(0, -1),
        glm::vec<2,int>(1, -1)
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
	void step(float delta_time);
    float calculate_density(Particle &particle);
    float sample_density_contribution(Particle& particle_j);
    float calculate_density_contribution(Particle& particle_i, Particle& particle_j);
    void sample_at_point(vec3 pos);
    void update_vertex_data(std::vector<GLfloat> *vertices);
    vec2 calculate_pressure_gradient(Particle &particle);
    vec3 calculate_pressure_contribution(Particle& particle_i, Particle& particle_j);
    float calculate_pressure(Particle& particle);
    vec2 calculate_pressue_acceleration(Particle& particle);
    vec3 calculate_viscosity_force(Particle& particle_i);
    vec3 calculate_viscosity_contribution(Particle& particle_i, Particle& particle_j);
    void handle_collisions(Particle& particle);
    glm::vec<2, int> get_cell_coord(vec2 pos);
    int hash_cell(glm::vec<2, int>  cell);
    int KeyFromHash(int hash, int tableSize);
    void update_spacial_lookup();
    template <typename R> R do_for_each_neighbour(Particle& particle_i, R(Simulation::*func)(Particle&,Particle&));
    void print_debug_info();
private:
    void create_walls();
   
};
