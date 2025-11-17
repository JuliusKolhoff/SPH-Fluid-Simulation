#pragma once
#define _USE_MATH_DEFINES
#include <vector>
#include <cmath>
#include "Rendering.h"

using namespace glm;
class Simulation
{
private:
    static Simulation* instance;
    Simulation() {};
public:
    const float particle_radius = 0.02f;
    const unsigned int simulation_size = 250;
	const float smoothing_radius = 1.0f;
    std::vector<Particle> particles;

    static Simulation& get_instance() {
        if (instance == nullptr) {
            instance = new Simulation();
        }
        return *instance;
    }
    ~Simulation() {
        delete instance;
	}
    void create_particles();
	void step(float delta_time);
    float W(vec3 particle_i_pos, vec3 particle_j_pos);
	float kernel_function(float r,float h);
    float calculate_density(Particle &particle);
    void update_vertex_data(std::vector<GLfloat> *vertices);
};
