#pragma once
#define _USE_MATH_DEFINES
#include <vector>
#include <cmath>
#include <random>
#include "Rendering.h"
#include "Particle.h"
#include <functional>

using namespace glm;
//hash 1 has 0ne added 5 at the end
static const int hashK1 = 158235;
static const int hashK2 = 9737333;

class Simulation
{
public:
    const float particle_radius = 16.0f;
    const float smoothing_radius = 1.0f;
    unsigned int simulation_size = 500;
    float target_density = 2.25f;
    float step_size = 0.1f;
    float k = 100.0f;
    Particle *particles = nullptr;
    std::pair<int, int> *spatial_lookup = nullptr;
    int *start_indices = nullptr;

private:
    static Simulation* instance;

    Simulation() {
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
    float W(vec3 particle_i_pos, vec3 particle_j_pos);
    float W1(vec3 particle_i_pos, vec3 particle_j_pos);
	float kernel_function(float r,float h);
    float kernel_function_derivative(float r, float h);
    float calculate_density(Particle &particle);
    float sample_density(vec3 pos);
    vec3 calculate_pressure_gradient(Particle &particle);
    void update_vertex_data(std::vector<GLfloat> *vertices);
    float calculate_pressure(Particle& particle);
    vec3 calculate_pressue_acceleration(Particle& particle);
    void handle_collisions(Particle& particle);
    glm::vec<2, int> get_cell_coord(vec2 pos);
    int hash_cell(glm::vec<2, int>  cell);
    int KeyFromHash(int hash, int tableSize);
    void update_spacial_lookup();
    template <typename R> R do_for_each_neighbour(vec2 pos, std::function<R(Particle&)>& func);
private:
    void create_walls();
   
};
