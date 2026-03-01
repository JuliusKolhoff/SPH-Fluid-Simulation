#include "Simulation.h"

Simulation* Simulation::instance = nullptr;
void Simulation::create_particles(){
    //Movables
	float distance_between_particles = 4 * particle_radius;
    for (unsigned int i = 0; i < simulation_size; i++) {
        float y = distance_between_particles * (0.5f * (int)sqrt(simulation_size) - (int)(i / (int)sqrt(simulation_size)));
        float x = distance_between_particles * (-0.5f * (int)sqrt(simulation_size) + (int)(i % (int)sqrt(simulation_size)));
        Particle particle = Particle(x, y, 0.0f, 1.0f, 1.0f, 1.0f);
        particles.push_back(particle);
    }
    //Walls
    for (unsigned int i = 0; i * particle_radius < 2; i+=4) {
        float y = 1;
        float x = i * particle_radius - 1;
        Particle particle = Particle(x, y, 0.0f, 1.0f, 0.0f, 0.0f);
        particle.movable = false;
        particles.push_back(particle);
        y = -1;
        x = i * particle_radius - 1;
        particle = Particle(x, y, 0.0f, 1.0f, 0.0f, 0.0f);
        particle.movable = false;
        particles.push_back(particle);
        x = 1;
        y = i * particle_radius - 1;
        particle = Particle(x, y, 0.0f, 1.0f, 0.0f, 0.0f);
        particle.movable = false;
        particles.push_back(particle);
        x = -1;
        y = i * particle_radius - 1;
        particle = Particle(x, y, 0.0f, 1.0f, 0.0f, 0.0f);
        particle.movable = false;
        particles.push_back(particle);
    }
}
void Simulation::step(float delta_time){
    float density_sum = 0.0f;
    for (auto& particle_i : particles) {
        calculate_density(particle_i);
        density_sum += particle_i.density;
        if (particle_i.movable) {
            particle_i.pos += (calculate_pressure_gradient(particle_i)-vec3(0.0f,0.1f,0.0f)) * 0.001f;
        }
    }

    //std::cout << "Density: " << density_sum/simulation_size << std::endl;
}
vec3 Simulation::calculate_pressure_gradient(Particle &particle_i){
	vec3 pressure_gradient(0.0f);
    if (particle_i.movable) {
        for (auto& particle_j : particles) {
            if (&particle_j != &particle_i) {
                vec3 direction = normalize(particle_i.pos - particle_j.pos);
                float contribution = W(particle_i.pos, particle_j.pos);
                pressure_gradient += particle_j.mass * ((particle_i.density / particle_j.density)) * contribution * direction;
            }
        }
    }
	return pressure_gradient;
}


//A Survey on SPH Methods in Computer Graphics Page 3: 2.4 Mass Density Estimation
float Simulation::calculate_density(Particle &particle_i) {
    float density = 0.0f;
    for (auto& particle_j : particles) {
        if (&particle_i != &particle_j) {
            float d = W(particle_i.pos, particle_j.pos);
            density += particle_j.mass * d;
			//particle_j.color = vec3(d, 0, 0);
			//std::cout << "Density Contribution from Particle: " << d << std::endl;
        }
    }
	particle_i.density = density;
    return density;
}
//Kernel function W between two particles
float Simulation::W(vec3 particle_i_pos, vec3 particle_j_pos) {
	return kernel_function(distance(particle_i_pos ,particle_j_pos) * 3, smoothing_radius);
}

//Smoothed Particle Hydrodynamics (SPH): an Overview and Recent Developments Page 12
float Simulation::kernel_function(float r,float h) {
    
	float R = (r / h);

    if (R > 2.0f) {
		return 0.0f;
    }
    //3D
	//float normal = (315.0f / 208.0f )*(PI * h * h * h);
    //2D
	float normal = 15.0f / (7.0f * M_PI * h * h);

    float r_2 = R * R;
	float r_3 = r_2 * R;
	float r_4 = r_3 * R;
    float value = normal * ((2.0f / 3.0f) - (9.0f / 8.0f) * r_2 + (19.0f / 24.0f) * r_3 - (5.0f / 32.0f) * r_4);
    return value;
}

void Simulation::update_vertex_data(std::vector<GLfloat> *vertices) {
    vertices->clear();
    for (const auto& particle : Simulation::get_instance().particles) {
        vertices->push_back(particle.pos.x);
        vertices->push_back(particle.pos.y);
        vertices->push_back(particle.pos.z);
        vertices->push_back(particle.color.r);
        vertices->push_back(particle.color.g);
        vertices->push_back(particle.color.b);
        //std::cout << "Particle Position: (" << particle.pos.x << ", " << particle.pos.y << ", " << particle.pos.z << ")\n";
    }
}