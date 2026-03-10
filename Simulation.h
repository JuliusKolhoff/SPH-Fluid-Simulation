#pragma once
#define _USE_MATH_DEFINES
#include <vector>
#include <cmath>
#include <random>
#include "Rendering.h"

using namespace glm;
class Simulation
{
public:
    const float particle_radius = 16.0f;
    const float smoothing_radius = 1.0f;
    unsigned int simulation_size = 500;
    float target_density = 1.0f;
    float step_size = 0.1f;
    float k = 100.0f;
    std::vector<Particle> particles;
    

private:
    static Simulation* instance;

    Simulation() {
        
    };
public:
    static Simulation& get_instance() {
        if (instance == nullptr) {
            instance = new Simulation();
        }
        return *instance;
    }
    ~Simulation() {
        
    }
    void create_particles();
    void random_particles();
    std::vector<Particle&> get_all_particles();
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
private:
    void create_walls();

};
