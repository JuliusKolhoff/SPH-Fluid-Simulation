#include "Simulation.h"

Simulation* Simulation::instance = nullptr;

void Simulation::step(float delta_time) {
    // CFL condition A Survey Sec.2.9:
    float v_max = 1.0f;   // avoid div/0
    for (int i = 0; i < simulation_size; ++i)
        v_max = max(v_max, length(vec2(particles[i].velocity)));

    const float lambda = 0.4f;
    const float h_px = smoothing_radius * 100.0f;   // kernel radius in screen px
    step_size = lambda * h_px / v_max;
    step_size = min(step_size, 0.016f);

    update_spacial_lookup();
    error_sum = 0;
    density_sum = 0;

    // 1 Density
    for (int i = 0; i < simulation_size; ++i) {
        Particle& p = particles[i];
        calculate_density(p);
        /*float ratio = p.density / target_density;
        p.color = vec3(ratio, 0.2f, 1.0f - ratio);*/
        density_sum += p.density;
    }
    //2 Pressure
    for (int i = 0; i < simulation_size; ++i) {
        Particle& p = particles[i];
        if (p.movable) {
            // Berechne Druck basierend auf Dichte-Fehler
            p.pressure = calculate_pressure(p);

        }
    }

    // 3 Forces
    for (int i = 0; i < simulation_size; ++i)
    {
        Particle& p = particles[i];
        if (p.movable) {
            vec2 accel = calculate_pressue_acceleration(p);
            accel += vec2(0, -gravity);
            //p.velocity += vec3(0, -10.0f,0);    // gravity
            p.velocity += vec3(accel, 0.0f) * step_size;
            p.velocity += calculate_viscosity_force(p) * step_size;
            //p.velocity *= 0.99f;
        }
    }

    //// 4 Integrate
    for (int i = 0; i < simulation_size; ++i)
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
    /*float  pressure = particle_i.density - (*instance).target_density;
    return k * max(0.0f,pressure);*/
    float  pressure = particle_i.density / (*instance).target_density;
    return max(0.0f, k * (powf(pressure, 7.0f) - 1.0f));
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
vec2 Simulation::calculate_pressure_gradient(Particle &particle_i){
	vec2 pressure_gradient(0.0f);
    if (particle_i.movable) {
        vec3 result = do_for_each_neighbour<vec3>(particle_i, &Simulation::calculate_pressure_contribution);
        error_sum += result.z;
        pressure_gradient = vec2(result.x, result.y);
    }
	return vec2(pressure_gradient);
}
vec3 Simulation::calculate_pressure_contribution(Particle& particle_i,Particle& particle_j) {
    float r = distance(vec2(particle_i.pos), vec2(particle_j.pos));
    if (r < 1e-6f) return vec3(0);
    vec2 direction = (vec2(particle_i.pos) - vec2(particle_j.pos))/r;
    float contribution = W1(particle_i.pos, particle_j.pos);
    vec2 acceleration = particle_j.mass * ((particle_i.pressure / (particle_i.density * particle_i.density)) + (particle_j.pressure / (particle_j.density * particle_j.density))) * contribution * direction;
    float error = particle_j.mass * ((1 / (particle_i.density * particle_i.density)) + (1 / (particle_j.density * particle_j.density))) * contribution;
    return vec3(acceleration.x, acceleration.y, error);
}
//A Survey on SPH Methods in Computer Graphics Page 6: 3.2 Local Pressure Solvers
vec2 Simulation::calculate_pressue_acceleration(Particle& particle_i) {
    vec2 pressure_gradient = calculate_pressure_gradient(particle_i);
    if (all(isnan((pressure_gradient)))) {
        pressure_gradient = vec2(0);
    }
    return -pressure_gradient;
}
//A Survey on SPH Methods in Computer Graphics Page 12: 5.1 Viscous Fluids
////A Survey on SPH Methods in Computer Graphics, Sec 5.1.2, Eq. 68
vec3 Simulation::calculate_viscosity_force(Particle& particle_i) {
    if (!particle_i.movable) return vec3(0);
    // For a 2-D simulation d=2, so the prefactor 2*(d+2) = 8.
    const float d = 2.0f;
    const float prefactor = 2.0f * (d + 2.0f);            // = 8
    return (viscosity/particle_i.density) * prefactor * do_for_each_neighbour<vec3>(particle_i, &Simulation::calculate_viscosity_contribution);

}
vec3 Simulation::calculate_viscosity_contribution(Particle& particle_i, Particle& particle_j) {
    // h in screen-space coordinates (kernel uses dist/100, smoothing_radius=0.5)
    const float h_screen = smoothing_radius * 100.0f;     // = 50 px
    const float eps = 0.01f * h_screen * h_screen;  // regularisation term

    vec3 x_ij = particle_i.pos - particle_j.pos;      // screen-space position diff
    vec3 v_ij = particle_i.velocity - particle_j.velocity;
    float r = length(x_ij);
    if (r < 1e-6f) return vec3(0);

    // Kernel gradient in screen-space (same convention as the pressure gradient):
    //   grad_W = W'(r/100, h_norm) * (x_ij / r)
    // W1 already returns the scalar derivative evaluated at the normalised distance.
    float W1_val = smoothing_kernel_derivative(r/100.0f,smoothing_radius);
    vec3  grad_W = W1_val * (x_ij / r);

    float dot_vx = dot(v_ij, x_ij);
    float denom = dot(x_ij, x_ij) + eps;

    return (particle_j.mass / particle_j.density) * (dot_vx / denom) * grad_W;
    
    /*float contribution = W2(particle_i.pos, particle_j.pos);
    vec3 vel_diff = particle_j.velocity - particle_i.velocity;
    return (particle_j.mass / particle_j.density) * vel_diff * contribution;*/
}


//A Survey on SPH Methods in Computer Graphics Page 3: 2.4 Mass Density Estimation
float Simulation::calculate_density(Particle& particle_i) {
    float density = do_for_each_neighbour<float>(particle_i,&Simulation::calculate_density_contribution);
    particle_i.density = density;
    return density;
}

float Simulation::sample_density_contribution(Particle& particle_j) {
    float d = W(sample_point, particle_j.pos);
    return particle_j.mass * d;
}

float Simulation::calculate_density_contribution(Particle& particle_i, Particle& particle_j) {
    float d = W(particle_i.pos, particle_j.pos);
    return particle_j.mass * d;
}
//Kernel function W between two particles
float Simulation::W(vec3 particle_i_pos, vec3 particle_j_pos) {
	//return kernel_function(distance(particle_i_pos ,particle_j_pos) * 3, smoothing_radius);
    //Smoothing radius from 0 to 1 but distance in screen space -> /100 to account for difference
    float dist = distance(particle_i_pos, particle_j_pos)/100;
	//return kernel_function(dist, smoothing_radius);
    //Lague
    return smoothing_kernel(dist, smoothing_radius);

}
float Simulation::W1(vec3 particle_i_pos, vec3 particle_j_pos) {
    //Smoothing radius from 0 to 1 but distance in screen space -> /100 to account for difference
    float dist = distance(particle_i_pos, particle_j_pos)/100.0f;
    //return kernel_function_derivative(dist, smoothing_radius);
    //Lague
    //return smoothing_kernel_derivative(dist, smoothing_radius);
    return spiky_kernel_derivative(dist, smoothing_radius);

}
float Simulation::W2(vec3 particle_i_pos, vec3 particle_j_pos) {
    //Smoothing radius from 0 to 1 but distance in screen space -> /100 to account for difference
    float dist = distance(particle_i_pos, particle_j_pos) / 100;
    //return kernel_function_derivative(dist, smoothing_radius);
    //Lague
    return smoothing_kernel_derivative_2(dist, smoothing_radius);

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
float Simulation::smoothing_kernel(float  dist, float radius) {
    if (dist > radius) return 0;
    float volume = M_PI * pow(radius, 8) / 4;
    float value = max(0.0f, radius * radius - dist * dist);
    return value * value * value / volume;
}
float Simulation::smoothing_kernel_derivative(float  dist, float radius) {
    if (dist > radius) return 0;
    float f = radius * radius -dist * dist;
    float scale = -24 / (M_PI * pow(radius, 8));
    return scale * dist * f * f;
}
float Simulation::smoothing_kernel_derivative_2(float  dist, float radius) {
    if (dist > radius) return 0.0f;

    float h2 = radius * radius;
    float f = h2 - dist * dist;

    float scale = -24.0f / (M_PI * pow(radius, 8));

    return scale * f * (h2 - 5.0f * dist * dist);
}
float Simulation::spiky_kernel(float dist, float radius) {
    if (dist > radius) return 0.0f;
    float h_r = radius - dist;
    float volume = M_PI * pow(radius, 5) / 10.0f;  // from 2-D normalisation above
    return (h_r * h_r * h_r) / volume;
}

float Simulation::spiky_kernel_derivative(float dist, float radius) {
    if (dist > radius) return 0.0f;
    float h_r = radius - dist;
    float scale = 30.0f / (M_PI * pow(radius, 5));
    return -scale * h_r * h_r;  // always ≤ 0, non-zero for r < h
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


void Simulation::handle_collisions(Particle& particle)
{
    
    float collisionDamping = 0.95f;

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
R Simulation::do_for_each_neighbour(Particle& particle_i, R(Simulation::*func)(Particle&, Particle&)) {
        // Find which cell the sample point is in (this will be the centre of our 3x3 block)
        glm::vec<2,int> centre = get_cell_coord(particle_i.pos);
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
                result += (this->*func)(particle_i,particle_j);
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
    start_y = (SCR_HEIGHT - block_h)  -  100;
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
void Simulation::sample_at_point(vec3 pos) {
    //float density = 0;
    /*for (auto& particle_j : particles) {
            float d = W(pos, particle_j.pos);
            if (d > 0)  particle_j.color = vec3(1, 0, 0); else particle_j.color = vec3(1,1,1);
            density += particle_j.mass * d;
    }*/
    /*std::function<float(Particle&)> func = [this, &pos](Particle& particle_j) {
        particle_j.color = {0,1.0,0.0 };
        float d = this->W(pos, particle_j.pos);
        return particle_j.mass * d;
        };*/
   /* std::function<float(Particle&)> func = [this, &pos](Particle& p2) {p2.color = { 0,1.0,0.0 }; if (distance(pos, p2.pos) < 6.0f) { return 1; } else return 0; };
    density = do_for_each_neighbour<float>(pos, func);
    return density;*/
    sample_point = pos;
    print_debug_info();

}
void Simulation::print_debug_info() {
    println("-----------------Debug-----------------");
    //Count Duplicates
    float duplicates = 0;
    for (int i = 0; i < simulation_size; ++i) {
        Particle& p = particles[i];
        glm::vec<2, int> centre = get_cell_coord(p.pos);
        for (glm::vec<2, int> offset : cell_offsets) {
            int key = KeyFromHash(hash_cell(centre + offset), simulation_size);
            if (key == -1) continue;
            int start = start_indices[key];
            for (int s = start; s < simulation_size; ++s) {
                if (spatial_lookup[s].second != key) break;
                Particle& p2 = particles[spatial_lookup[s].first];
                if (&p != &p2 && distance(p.pos, p2.pos) < 6.0f) duplicates++;
            }
        }
    }
    println("Duplicates: " << duplicates / 2);
    //Avg Error
    println("Avg. Error: "  << error_sum / simulation_size);
    //Avg Density
    println("Avg. Density: "  << density_sum / simulation_size);
    
    println("");
    
    //Sample
    float density = 0;
        glm::vec<2, int> centre = get_cell_coord(vec2(sample_point));
        for (glm::vec<2, int> offset : cell_offsets) {
            int key = KeyFromHash(hash_cell(centre + offset), simulation_size);
            if (key == -1) continue;
            int start = start_indices[key];
            for (int s = start; s < simulation_size; ++s) {
                if (spatial_lookup[s].second != key) break;
                Particle& p2 = particles[spatial_lookup[s].first];
                density += sample_density_contribution(p2);
            }
        }
    println("Sample Density: " << density);

    println("---------------------------------------");
}
