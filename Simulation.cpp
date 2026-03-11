#include "Simulation.h"

Simulation* Simulation::instance = nullptr;

void Simulation::step(float delta_time){
    //step_size = delta_time;
    delta_time = max(delta_time, 0.016f);
    update_spacial_lookup();

    
            Particle& p = particles[20];
            std::function<int(Particle&)> func = [this, p](Particle& particle_j) {
                particle_j.color = { 1.0,0,0 };
                return 0;
            };
            int x = do_for_each_neighbour<int>(p.pos, func);
            
   
    


    // 1 Density
    for (int i = 0;i<simulation_size;++i) {
        Particle& p = particles[i];
        calculate_density(p);
        float ratio = p.density / target_density;
        p.color = vec3(ratio, 0.2f, 1.0f - ratio);
        
    }
        for (int i = 0;i<simulation_size;++i) {
            Particle& p = particles[i];
            if (p.movable) {
                // Berechne Druck basierend auf Dichte-Fehler
                p.pressure = calculate_pressure(p);

            }
        }

    //// 3 Forces
    for (int i = 0;i<simulation_size;++i)
    {
        Particle& p = particles[i];
        if (p.movable) {
            vec3 accel = calculate_pressue_acceleration(p);
            //accel += vec3(0, -0.5f * k, 0);    // gravity
            p.velocity += accel * step_size;
            p.velocity *= 0.99f;
        }
    }

    //// 4 Integrate
    for (int i = 0;i<simulation_size;++i)
    {
        Particle& p = particles[i];
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
	vec2 pressure_gradient(0.0f);
    float error = 0;
    if (particle_i.movable) {
        //for (auto& particle_j : particles) {
        //    if (&particle_j != &particle_i) {
        //        vec3 direction = normalize(particle_i.pos - particle_j.pos);
        //        float contribution = W1(particle_i.pos, particle_j.pos);
        //        pressure_gradient += particle_j.mass * ((particle_i.pressure / (particle_i.density * particle_i.density)) + (particle_j.pressure / (particle_j.density * particle_j.density))) * contribution * direction;
        //        //Error
        //        error += particle_j.mass * ((1 / (particle_i.density * particle_i.density)) + (1 / (particle_j.density * particle_j.density))) * contribution;
        //    }
        //}
        std::function<vec3(Particle&)> func = [this, particle_i](Particle& particle_j) {
            if (particle_i.pos == particle_j.pos) return vec3(0);
            vec3 direction = normalize(particle_i.pos - particle_j.pos);
            float contribution = W1(particle_i.pos, particle_j.pos);
            return particle_j.mass * ((particle_i.pressure / (particle_i.density * particle_i.density)) + (particle_j.pressure / (particle_j.density * particle_j.density))) * contribution * direction;
           };
        pressure_gradient = do_for_each_neighbour<vec3>(particle_i.pos, func);
    }
    //print(error);
	return vec3(pressure_gradient,0.0f);
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
    std::function<float(Particle&)> func  = [this, particle_i](Particle& particle_j) {
        float d = this->W(particle_i.pos, particle_j.pos);
        return particle_j.mass * d; 
    };
    float density = do_for_each_neighbour<float>(particle_i.pos,func);
    /*for (auto& particle_j : particles) {
        float d = W(particle_i.pos, particle_j.pos);
        density += particle_j.mass * d;
    }*/
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
    for (int i = 0; i < simulation_size;i++) {
        vertices->push_back(particles[i].pos.x);
        vertices->push_back(particles[i].pos.y);
        vertices->push_back(particles[i].pos.z);
        vertices->push_back(particles[i].color.r);
        vertices->push_back(particles[i].color.g);
        vertices->push_back(particles[i].color.b);
        //std::cout << "Particle Position: (" << particle.pos.x << ", " << particle.pos.y << ", " << particle.pos.z << ")\n";
    }
}

float Simulation::sample_density(vec3 pos) {
    float density = 0;
    /*for (auto& particle_j : particles) {
            float d = W(pos, particle_j.pos);
            if (d > 0)  particle_j.color = vec3(1, 0, 0); else particle_j.color = vec3(1,1,1);
            density += particle_j.mass * d;
    }*/
    std::function<int(Particle&)> func = [this](Particle& particle_j) {
        particle_j.color = { 0,1.0,0.0 };
        return 0;
        };
    int x = do_for_each_neighbour<int>(pos, func);
    return density;
}

void Simulation::handle_collisions(Particle& particle)
{
    
    float collisionDamping = 0.85f;

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
void Simulation::update_spacial_lookup()
{
    for (int i = 0; i < (int)simulation_size; ++i) {
        Particle& particle = particles[i];

        vec<2,int> cell = get_cell_coord(particle.pos);
        int hash = hash_cell(cell);
        int key = KeyFromHash(hash, simulation_size);
        spatial_lookup[i] = std::make_pair(i, key);
        start_indices[i] = INT_MAX;
    }

    auto compare = [](const std::pair<int, int>& a, const std::pair<int, int>& b) {
        return a.second > b.second;
    };

    std::sort(spatial_lookup, spatial_lookup + simulation_size, compare);

    for (int i = 0; i < (int)simulation_size; ++i) {
        int key = spatial_lookup[i].second;
        int prev_key = i == 0 ? INT_MAX : spatial_lookup[i - 1].second;
        if (key != prev_key) {
            start_indices[key] = i;
        }
    }
}
glm::vec<2, int> Simulation::get_cell_coord(vec2 pos) {
    int file = static_cast<int>(pos.x / (smoothing_radius * 100.0f));
    int rank = static_cast<int>(pos.y / (smoothing_radius * 100.0f));

    return glm::vec<2, int>(file, rank);
}
int Simulation::hash_cell(glm::vec<2, int> cell)
{
    int grid_width = SCR_WIDTH / (smoothing_radius * 100.0f);
    int grid_height = SCR_HEIGHT / (smoothing_radius * 100.0f);

    //Bounds Check
    if (cell.x < 0 || cell.x > grid_width || cell.y < 0 || cell.y > grid_height) return -1;

    int a = cell.x * hashK1;
    int b = cell.y * hashK2;
    return (a + b);
}

int Simulation::KeyFromHash(int hash, int tableSize)
{
    //hash out of bound -> no key
    if (hash < 0) return -1;
    return hash % tableSize;
}
template <typename R> 
R Simulation::do_for_each_neighbour(vec2 pos, std::function<R(Particle&)>& func) {
        // Find which cell the sample point is in (this will be the centre of our 3x3 block)
        glm::vec<2,int> centre = get_cell_coord(pos);
        R result = R{};
        // Loop over all cells of the 3x3 block around the centre cell
        for (glm::vec<2, int> offset : cell_offsets)
        {
            // Get key of current cell, then loop over all points that share that key
            int key = KeyFromHash(hash_cell(centre+offset),simulation_size);
            if (key == -1) continue;
            int cellStartIndex = start_indices[key];

            for (int i = cellStartIndex; i < simulation_size; i++)
            {
                // Exit loop if we're no longer looking at the correct cell
                if (spatial_lookup[i].second != key) break;

                int particleIndex = spatial_lookup[i].first;
                Particle& particle_j = particles[particleIndex];
                    //float sqrDst = glm::length2(points[particleIndex] - samplePoint);

                    //// Test if the point is inside the radius
                    //if (sqrDst <= sqrRadius)
                    //{
                    //    // Do something with the particleIndex!
                    //    // (either by writing code here that uses it directly, or more likely by
                    //    // having this function take in a callback, or return a vector, etc.)
                    //}
                result += func(particle_j);
            }
        }
        return result;
}
void Simulation::create_particles(){
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
        particles[i].pos.x = start_x + (i % cols) * diameter;
        particles[i].pos.y = start_y + (i / cols) * diameter;
        particles[i].color = { 1.0,1.0,1.0 };
    }
    update_spacial_lookup();
    //create_walls();

}
void Simulation::random_particles() {
    srand(time(0));
    for (unsigned int i = 0; i < simulation_size; i++) {
        particles[i].pos.x = ((float)rand() / RAND_MAX) * SCR_WIDTH;
        particles[i].pos.y = ((float)rand() / RAND_MAX) * SCR_HEIGHT;
        particles[i].color = { 1.0,1.0,1.0 };
    }
    update_spacial_lookup();
    //create_walls();

}

void Simulation::create_walls() {
    for (unsigned int i = 0; i * particle_radius <= SCR_WIDTH; i += 2) {
        /*float y = 1;
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
        particles.push_back(particle);*/
    }
}