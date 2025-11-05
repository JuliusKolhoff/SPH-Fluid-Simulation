#include "Simulation.h"

Simulation* Simulation::instance = nullptr;
void Simulation::create_particles(){
    for (unsigned int i = 0; i < simulation_size; i++) {
        float y = 2 * particle_radius * (0.5f * (int)sqrt(simulation_size) - (int)(i / (int)sqrt(simulation_size)));
        float x = 2 * particle_radius * (-0.5f * (int)sqrt(simulation_size) + (int)(i % (int)sqrt(simulation_size)));
        Particle particle = Particle(x, y, 0.0f, 1.0f, 1.0f, 1.0f);
        particles.push_back(particle);
    }
}
void Simulation::step(float delta_time){
    for (auto& particle_i : particles) {
        int count = 0;
        for (auto& particle_j : particles) {
            if (&particle_i != &particle_j) {
				count++;
            }
        }
    }
}

float Simulation::calculate_density(Particle particle_i) {
	float density = 0.0f;
    for (auto& particle_j : particles) {
        if (&particle_i != &particle_j) {
            
        }
    }
    return density;
}
//Kernel function W between two particles
float Simulation::W(vec3 particle_i_pos, vec3 particle_j_pos) {
	return kernel_function(length(particle_i_pos - particle_j_pos), smoothing_radius);
}

//Smoothed Particle Hydrodynamics (SPH): an Overview and Recent Developments Page 12
float Simulation::kernel_function(float r,float h) {
    
	float R = r / h;
    if (R > 2.0f) {
		return 0.0f;
    }
    //3D
	//float normal = (315.0f / 208.0f )*(PI * h * h * h);
    // 
    //2D
	float normal = 15.0f / (7.0f * M_PI * h * h);
    float r_2 = R * R;
	float r_3 = r_2 * R;
	float r_4 = r_3 * R;
    float value = normal * ((2.0f / 3.0f) - (9.0f / 8.0f) * r_2 + (19.0f / 24.0f) * r_3 - (5.0f / 32.0f) * r_4);
    return value;
}