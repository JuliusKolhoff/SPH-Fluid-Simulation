#include "Simulation.h"

Simulation* Simulation::instance = nullptr;

void Simulation::step(float delta_time){
    //step_size = delta_time;
    delta_time = max(delta_time, 0.016f);
    // 1 Density
    for (auto& p : particles) {
        calculate_density(p);
        float ratio = p.density / target_density;
        p.color = vec3(ratio, 0.2f, 1.0f - ratio);
        
    }
        for (auto& p : particles) {
            if (p.movable) {
                // Berechne Druck basierend auf Dichte-Fehler
                p.pressure = calculate_pressure(p);
            }
        }

    // 3 Forces
    for (auto& p : particles)
    {
        if (p.movable) {
            vec3 accel = calculate_pressue_acceleration(p);
            //accel += vec3(0, -0.5f * k, 0);    // gravity
            p.velocity += accel * step_size;
            p.velocity *= 0.99f;
        }
    }

    // 4 Integrate
    for (auto& p : particles)
    {
        if (p.movable)
        {
            p.pos += p.velocity * step_size;
            handle_collisions(p);
        }
    }
    //std::cout << "Density: " << density_sum/simulation_size << std::endl;
}
//A Survey on SPH Methods in Computer Graphics Page 6: 3.1. Measuring Compression
float Simulation::calculate_pressure(Particle& particle_i) {
    float  pressure = particle_i.density - (*instance).target_density;
    return k * max(0.0f,pressure);
    

}
//A Survey on SPH Methods in Computer Graphics Page 6: 3.2 Local Pressure Solvers  /  Page 4 Eqation(12)
//vec3 Simulation::calculate_pressure_gradient(Particle &particle_i){
//	vec3 pressure_gradient(0.0f);
//    if (particle_i.movable) {
//        for (auto& particle_j : particles) {
//            if (&particle_j != &particle_i) {
//                vec3 direction = normalize(particle_i.pos - particle_j.pos);
//                float contribution = W1(particle_i.pos, particle_j.pos);
//                pressure_gradient += particle_j.mass * ((particle_i.density / particle_j.density)) * contribution * direction;
//            }
//        }
//    }
//    print(pressure_gradient.x);
//	return -pressure_gradient;
//}
// 
//A Survey on SPH Methods in Computer Graphics Page 6: 3.2 Local Pressure Solvers  /  Page 4 Eqation(12)
vec3 Simulation::calculate_pressure_gradient(Particle &particle_i){
	vec3 pressure_gradient(0.0f);
    float error = 0;
    if (particle_i.movable) {
        for (auto& particle_j : particles) {
            if (&particle_j != &particle_i) {
                vec3 direction = normalize(particle_i.pos - particle_j.pos);
                float contribution = W1(particle_i.pos, particle_j.pos);
                pressure_gradient += particle_j.mass * ((particle_i.pressure / (particle_i.density * particle_i.density)) + (particle_j.pressure / (particle_j.density * particle_j.density))) * contribution * direction;
                //Error
                error += particle_j.mass * ((1 / (particle_i.density * particle_i.density)) + (1 / (particle_j.density * particle_j.density))) * contribution;
            }
        }
    }
    //print(error);
	return pressure_gradient;
}
//A Survey on SPH Methods in Computer Graphics Page 6: 3.2 Local Pressure Solvers
vec3 Simulation::calculate_pressue_acceleration(Particle& particle_i) {
    vec3 pressure_gradient = calculate_pressure_gradient(particle_i);
    if (all(isnan((pressure_gradient)))) {
        pressure_gradient = vec3(0);
    }
    return -pressure_gradient;
}


//A Survey on SPH Methods in Computer Graphics Page 3: 2.4 Mass Density Estimation
float Simulation::calculate_density(Particle& particle_i) {
    float density = 0;
    for (auto& particle_j : particles) {
        float d = W(particle_i.pos, particle_j.pos);
        density += particle_j.mass * d;
    }
    particle_i.density = density;
    return density;
}
//Kernel function W between two particles
float Simulation::W(vec3 particle_i_pos, vec3 particle_j_pos) {
	//return kernel_function(distance(particle_i_pos ,particle_j_pos) * 3, smoothing_radius);
    //Smoothing radius from 0 to 1 but distance in screen space -> /100 to account for difference
    float dist = distance(particle_i_pos, particle_j_pos)/100;
	return kernel_function(dist, smoothing_radius);
}
float Simulation::W1(vec3 particle_i_pos, vec3 particle_j_pos) {
    //Smoothing radius from 0 to 1 but distance in screen space -> /100 to account for difference
    float dist = distance(particle_i_pos, particle_j_pos)/100;
    return kernel_function_derivative(dist, smoothing_radius);
    
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
	float normal = 15.0f / (7.0f * M_PI * h  * h);

    float r_2 = R * R;
	float r_3 = r_2 * R;
	float r_4 = r_3 * R;
    float value = normal * ((2.0f / 3.0f) - (9.0f / 8.0f) * r_2 + (19.0f / 24.0f) * r_3 - (5.0f / 32.0f) * r_4);
    if (value < 0) value = 0;
    return value;
}
float Simulation::kernel_function_derivative(float r, float h)
{
    float R = r / h;

    if (R > 2.0f) {
        return 0.0f;
    }

    // 2D normalization
    float normal = 15.0f / (7.0f * M_PI * h * h);

    float r2 = R * R;
    float r3 = r2 * R;

    float value = (normal / (h*h)) *
        (-(9.0f / 4.0f) * R +
            (19.0f / 8.0f) * r2 -
            (5.0f / 8.0f) * r3);

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

float Simulation::sample_density(vec3 pos) {
    float density = 0;
    for (auto& particle_j : particles) {
            float d = W(pos, particle_j.pos);
            if (d > 0)  particle_j.color = vec3(1, 0, 0); else particle_j.color = vec3(1,1,1);
            density += particle_j.mass * d;
    }
    return density;
}

void Simulation::handle_collisions(Particle& particle)
{
    
    float collisionDamping = 0.55f;

    vec3 pos = particle.pos;
    vec3 vel = particle.velocity;


    if (pos.x <= 0)
    {
        pos.x = 0;
        vel.x *= -1 * collisionDamping;
    }
    if (pos.x >= SCR_WIDTH) {
        pos.x = SCR_WIDTH;
        vel.x *= -1 * collisionDamping;
    }

    if (pos.y <= 0)
    {
        pos.y = 0;
        vel.y *= -1 * collisionDamping;
    }
    if (pos.y >= SCR_HEIGHT) {
        pos.y = SCR_HEIGHT;
        vel.y *= -1 * collisionDamping;
    }

    // Collide particle against the test obstacle
   /* const vec3 obstacleHalfSize = obstacleSize * 0.5;
    vec3 obstacleEdgeDst = obstacleHalfSize - abs(pos - obstacleCentre);

    if (obstacleEdgeDst.x >= 0 && obstacleEdgeDst.y >= 0)
    {
        if (obstacleEdgeDst.x < obstacleEdgeDst.y) {
            pos.x = obstacleHalfSize.x * sign(pos.x - obstacleCentre.x) + obstacleCentre.x;
            vel.x *= -1 * collisionDamping;
        }
        else {
            pos.y = obstacleHalfSize.y * sign(pos.y - obstacleCentre.y) + obstacleCentre.y;
            vel.y *= -1 * collisionDamping;
        }
    }*/

    // Update position and velocity
    particle.pos = pos;
    particle.velocity = vel;
}
void Simulation::create_particles(){
    particles.clear();

    const float diameter = particle_radius * 2.0f;

    int cols = (int)std::ceil(std::sqrt((float)simulation_size));
    int rows = (int)std::ceil((float)simulation_size / cols);

    float block_w = cols * diameter;
    float block_h = rows * diameter;

    // Centre 
    float start_x = (SCR_WIDTH - block_w) / 2.0f + particle_radius;
    float start_y = (SCR_HEIGHT - block_h) / 2.0f + particle_radius;
    start_y = glm::max(particle_radius, start_y);

    for (unsigned int i = 0; i < simulation_size; i++) {
        float x = start_x + (i % cols) * diameter;
        float y = start_y + (i / cols) * diameter;
        particles.push_back(Particle(x, y, 0.0f, 1.0f, 1.0f, 1.0f));
    }
    //create_walls();

}
void Simulation::random_particles() {
    srand(time(0));
    for (unsigned int i = 0; i < simulation_size; i++) {
        float x = ((float)rand() / RAND_MAX) * SCR_WIDTH;
        float y = ((float)rand() / RAND_MAX) * SCR_HEIGHT;
        Particle particle = Particle(x, y, 0.0f, 1.0f, 1.0f, 1.0f);
        particles.push_back(particle);
    }
    //create_walls();

}

void Simulation::create_walls() {
    for (unsigned int i = 0; i * particle_radius <= SCR_WIDTH; i += 2) {
        float y = 1;
        float x = i * particle_radius;
        Particle particle = Particle(x, y, 0.0f, 1.0f, 0.0f, 0.0f);
        particle.movable = false;
        particles.push_back(particle);
        y = SCR_HEIGHT;
        x = i * particle_radius;
        particle = Particle(x, y, 0.0f, 1.0f, 0.0f, 0.0f);
        particle.movable = false;
        particles.push_back(particle);
        x = SCR_WIDTH;
        y = i * particle_radius;
        particle = Particle(x, y, 0.0f, 1.0f, 0.0f, 0.0f);
        particle.movable = false;
        particles.push_back(particle);
        x = 0;
        y = i * particle_radius;
        particle = Particle(x, y, 0.0f, 1.0f, 0.0f, 0.0f);
        particle.movable = false;
        particles.push_back(particle);
    }
}