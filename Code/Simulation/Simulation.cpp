#include "Simulation.h"

Simulation* Simulation::instance = nullptr;

void Simulation::step(float delta_time,GLFWwindow* window) {
    // CFL condition A Survey Sec.2.9:
    float v_max = 1.0f;   // avoid div/0
    for (int i = 0; i < simulation_size; ++i)
        v_max = max(v_max, length(vec2(particles[i].velocity)));

    const float lambda = 0.4f;
    const float h_px = smoothing_radius;   // kernel radius in screen px

    step_size = lambda * h_px / v_max;
    //step_size = min(step_size, delta_time);
    step_size = min(step_size, 0.016f); // safety
    update_spacial_lookup();
    error_sum = 0;
    density_sum = 0;

    // 1 Density
    float sum = 0;
#pragma omp parallel for reduction(+:sum) schedule(dynamic, 32)
    for (int i = 0; i < simulation_size  +  obstacle_particle_count; ++i) {
        Particle& p = particles[i];
        calculate_density(p);
        
        /*float ratio = p.density / target_density;
        p.color = vec3(1.0f - ratio,1.0f  - ratio, 1.0f);*/
        sum += p.density;
    }
    density_sum = sum;
#pragma omp parallel for schedule(static)
    //2 Pressure
    for (int i = 0; i < simulation_size + obstacle_particle_count; ++i) {
        Particle& p = particles[i];
        if (p.movable) {
            // Berechne Druck basierend auf Dichte-Fehler
            p.pressure = calculate_pressure(p);

        }
    }
    // 3 Forces
    double x, y;//Mouse POs
    glfwGetCursorPos(window, &x, &y);
    y = SCR_HEIGHT - y;
#pragma omp parallel for schedule(dynamic, 32)
    for (int i = 0; i < simulation_size + obstacle_particle_count; ++i)
    {
        Particle& p = particles[i];
        if (p.movable) {
            vec2 accel = calculate_pressue_acceleration(p);
            accel += vec2(0, -gravity);
            //p.velocity += vec3(0, -10.0f,0);    // gravity
            p.velocity += vec3(accel, 0.0f) * step_size;
            p.velocity += calculate_viscosity_force(p) * step_size;
            //p.velocity *= 0.99f;
            
            
           /* float speed = length(vec2(p.velocity));
            float v_max = 1000.0f;
            float ratio = clamp(speed / v_max, 0.0f, 1.0f);
            ratio /= 2;*/
            //p.color = vec3(ratio, ratio, 1.0f);
            //COlor Coading
            float speed = length(vec2(p.velocity));
            float max_speed = 600.0f;
            float t = clamp(speed / max_speed, 0.0f, 1.0f); 
            //blue (slow) -> cyan -> white (fast)
            vec3 slow_color = vec3(0.05f, 0.15f, 0.6f);
            vec3 fast_color = vec3(0.9f, 0.95f, 1.0f);
            //vec3 fast_color = vec3(0.9f, 0.5f, 0.5f);
            p.color = mix(slow_color, fast_color, t);

            vec2 externalForces = vec2(0);
            float range = 100;
            //float force = k * 0.05f;
            float force = -k * 0.05f;
            if (right_mouse_down)
            {
                force_point = vec3(x, y, 0.0f);
                vec2 moveToMouse = force_point - p.pos;
                if (length(moveToMouse) < range)
                {
                    /*if (Input.GetMouseButton(1))
                    {
                        moveToMouse *= -1;
                    }*/
                    p.velocity += vec3((moveToMouse/length(moveToMouse)) * force,0.0f);
                }
            }
        }
    }
    


#pragma omp parallel for schedule(static)
    //// 4 Integrate
    for (int i = 0; i < simulation_size + obstacle_particle_count; ++i)
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
    return max(-INFINITY, k * (powf(pressure, 7.0f) - 1.0f));
}

//A Survey on SPH Methods in Computer Graphics Page 6: 3.2 Local Pressure Solvers  /  Page 4 Eqation(12)
vec2 Simulation::calculate_pressure_gradient(Particle& particle_i) {
    vec2 pressure_gradient(0.0f);
    if (particle_i.movable) {
        vec3 result = do_for_each_neighbour<vec3>(particle_i, &Simulation::calculate_pressure_contribution);
        //error_sum += result.z;
        pressure_gradient = vec2(result.x, result.y);
    }
    return vec2(pressure_gradient);
}
vec3 Simulation::calculate_pressure_contribution(Particle& particle_i, Particle& particle_j) {
    float r = distance(vec2(particle_i.pos), vec2(particle_j.pos));
    if (r < 1e-6f) return vec3(0);
    vec2 direction = (vec2(particle_i.pos) - vec2(particle_j.pos)) / r;
    float contribution = kernel_manager.W1_pressure(r,smoothing_radius);
    vec2 acceleration = particle_j.mass * ((particle_i.pressure / (particle_i.density * particle_i.density)) + (particle_j.pressure / (particle_j.density * particle_j.density))) * contribution * direction;
    float error = particle_j.mass * ((1 / (particle_i.density * particle_i.density)) + (1 / (particle_j.density * particle_j.density))) * contribution;
    return vec3(acceleration.x, acceleration.y, error);
}
//A Survey on SPH Methods in Computer Graphics Page 6: 3.2 Local Pressure Solvers
vec2 Simulation::calculate_pressue_acceleration(Particle& particle_i) {
    vec2 pressure_gradient = calculate_pressure_gradient(particle_i);
    if (any(isnan((pressure_gradient)))) {
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
    float safe_density = std::max(particle_i.density, 1e-4f);
    return (viscosity/safe_density) * prefactor * do_for_each_neighbour<vec3>(particle_i, &Simulation::calculate_viscosity_contribution);

}
vec3 Simulation::calculate_viscosity_contribution(Particle& particle_i, Particle& particle_j) {
    const float h_screen = smoothing_radius;     
    const float eps = 0.01f * h_screen * h_screen;  // regularisation term

    vec3 x_ij = particle_i.pos - particle_j.pos;      // screen-space position diff
    vec3 v_ij = particle_i.velocity - particle_j.velocity;
    float r = length(x_ij);
    if (r < 1e-6f) return vec3(0);

    // Kernel gradient in screen-space (same convention as the pressure gradient):
    //   grad_W = W'(r/100, h_norm) * (x_ij / r)
    // W1 already returns the scalar derivative evaluated at the normalised distance.
    float W1_val = kernel_manager.W1_viscosity(r,smoothing_radius);
    vec3  grad_W = W1_val * (x_ij / r);

    float dot_vx = dot(v_ij, x_ij);
    float denom = dot(x_ij, x_ij) + eps;

    return (particle_j.mass / particle_j.density) * (dot_vx / denom) * grad_W;
    
}


//A Survey on SPH Methods in Computer Graphics Page 3: 2.4 Mass Density Estimation
float Simulation::calculate_density(Particle& particle_i) {
    float density = do_for_each_neighbour<float>(particle_i,&Simulation::calculate_density_contribution);
    particle_i.density = density;
    if (density == 0)
        std::cout << particle_i.pos.x << "," << particle_i.pos.y << std::endl;
    return density;
}

float Simulation::sample_density_contribution(Particle& particle_j) {
    float dist = distance(sample_point, particle_j.pos);
    float d = kernel_manager.W_density(dist, smoothing_radius);
    return particle_j.mass * d;
}

float Simulation::calculate_density_contribution(Particle& particle_i, Particle& particle_j) {
    float dist = distance(particle_i.pos, particle_j.pos);
    float d = kernel_manager.W_density(dist, smoothing_radius);
    return particle_j.mass * d;
}

void Simulation::update_vertex_data(std::vector<GLfloat> *vertices) {
    vertices->clear();
    for (int i = 0; i < simulation_size + obstacle_particle_count;i++) {
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
    for (int i = 0; i < (int)simulation_size + obstacle_particle_count; ++i) {
        Particle& particle = particles[i];

        vec<2,int> cell = get_cell_coord(particle.pos);
        int hash = hash_cell(cell);
        int key = KeyFromHash(hash, simulation_size + obstacle_particle_count);
        spatial_lookup[i] = std::make_pair(i, key);
        start_indices[i] = INT_MAX;
    }

    auto compare = [](const std::pair<int, int>& a, const std::pair<int, int>& b) {
        return a.second > b.second;
    };

    std::sort(spatial_lookup, spatial_lookup + simulation_size + obstacle_particle_count, compare);

    for (int i = 0; i < (int)simulation_size + obstacle_particle_count; ++i) {
        int key = spatial_lookup[i].second;
        int prev_key = i == 0 ? INT_MAX : spatial_lookup[i - 1].second;
        if (key != prev_key) {
            if (key >= 0 && key < (int)(simulation_size + obstacle_particle_count)) {
                start_indices[key] = i;
            }
        }
    }
}
glm::vec<2, int> Simulation::get_cell_coord(vec2 pos) {
    int file = static_cast<int>(pos.x / (smoothing_radius));
    int rank = static_cast<int>(pos.y / (smoothing_radius));

    return glm::vec<2, int>(file, rank);
}
int Simulation::hash_cell(glm::vec<2, int> cell)
{
    int grid_width = SCR_WIDTH / (smoothing_radius);
    int grid_height = SCR_HEIGHT / (smoothing_radius);

    //Bounds Check
    if (cell.x < 0 || cell.x > grid_width || cell.y < 0 || cell.y > grid_height) return -1;

    int a = cell.x * hashK1;
    int b = cell.y * hashK2;
    return (a + b); 
    /*uint32_t a = static_cast<uint32_t>(cell.x) * 92837111u;
    uint32_t b = static_cast<uint32_t>(cell.y) * 689287499u;
    return a ^ b;*/
}

int Simulation::KeyFromHash(int hash, int tableSize)
{
    //hash out of bound -> no key
    if (hash < 0) return -1;
    return hash % tableSize;
    /*if (hash == UINT32_MAX) return -1;
    return static_cast<int>(hash % static_cast<uint32_t>(tableSize));*/
}
template <typename R> 
R Simulation::do_for_each_neighbour(Particle& particle_i, R(Simulation::*func)(Particle&, Particle&)) {

        glm::vec<2,int> centre = get_cell_coord(particle_i.pos);
        R result = R{};

        for (glm::vec<2, int> offset : cell_offsets)
        {
            // Get key of current cell
            int key = KeyFromHash(hash_cell(centre+offset),simulation_size + obstacle_particle_count);
            if (key == -1) continue;
            int cellStartIndex = start_indices[key];
            //Loop all Neighbouts
            for (int i = cellStartIndex; i < simulation_size + obstacle_particle_count; i++)
            {
                //BReak if no longer right key
                if (spatial_lookup[i].second != key) break;

                int particleIndex = spatial_lookup[i].first;
                Particle& particle_j = particles[particleIndex];
                //float sqrDst = glm::length2(points[particleIndex] - samplePoint);
                
                //if (sqrDst <= sqrRadius)
                //{
                    result += (this->*func)(particle_i, particle_j);
                //}

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
    start_y = (SCR_HEIGHT - block_h);
    start_y = glm::max(particle_radius, start_y);

    //start_x = 0;

    for (unsigned int i = 0; i < simulation_size; i++) {
        particles[i].pos.x = start_x + (i % cols) * diameter;
        particles[i].pos.y = start_y + (i / cols) * diameter;
        particles[i].color = base_particle_color;
    }
    create_obstacles();  
    update_spacial_lookup();

}
void Simulation::random_particles() {
    srand(time(0));
    for (unsigned int i = 0; i < simulation_size; i++) {
        particles[i].pos.x = ((float)rand() / RAND_MAX) * SCR_WIDTH;
        particles[i].pos.y = ((float)rand() / RAND_MAX) * SCR_HEIGHT;
        particles[i].color = base_particle_color;
    }
    create_obstacles();
    update_spacial_lookup();

}

void Simulation::create_obstacles() {
    int count = 0;
    float frequency = 1.0f;
    bool  fill = true;
    int max_fill_depth = scale;
    //---SLOPE---
    /*for (unsigned int i = 0; i * particle_radius <= SCR_WIDTH &&  count < obstacle_particle_count; i+= frequency) {
        float x = i * particle_radius;
        float y = (SCR_HEIGHT / 2) - (i / 2.0f) * particle_radius;
        if (y < 0) break;
        if (fill) {
            for (int j = 0; y > 0 && count < obstacle_particle_count;j++) {
                y -= frequency * particle_radius;
                particles[simulation_size + count].pos = vec3(x, y, 0.0f);
                particles[simulation_size + count].color = obstacle_color;
                particles[simulation_size + count].movable = false;
                count++;
            }
        }
        else {
            particles[simulation_size + count].pos = vec3(x, y, 0.0f);
            particles[simulation_size + count].color = obstacle_color;
            particles[simulation_size + count].movable = false;
            count++;
        }
    }*/


    //---SPIKE---
    /*float height = 400;
    float x = -particle_radius, y = particle_radius - (height / (SCR_WIDTH / 2)) * particle_radius;
    for (unsigned int i = 0; i * particle_radius <= SCR_WIDTH + particle_radius && count < obstacle_particle_count; i += frequency) {
        x += particle_radius;
        if (i * particle_radius <= SCR_WIDTH / 2) {
            y += (height/(SCR_WIDTH / 2)) * particle_radius;
        }
        else {
            y -= (height/(SCR_WIDTH / 2)) * particle_radius;
        }
        
        if (y < 0) break;
        if (fill) {
            float old_y = y;
            for (int j = 0; j < max_fill_depth && y > 0 && count < obstacle_particle_count; j++) {
                y -= frequency * particle_radius;
                particles[simulation_size + count].pos = vec3(x, y, 0.0f);
                particles[simulation_size + count].color = obstacle_color;
                particles[simulation_size + count].movable = false;
                count++;
            }
            y = old_y;
        }
        else {
            particles[simulation_size + count].pos = vec3(x, y, 0.0f);
            particles[simulation_size + count].color = obstacle_color;
            particles[simulation_size + count].movable = false;
            count++;
        }
    }*/
    obstacle_particle_count = count;

    //for (unsigned int i = 0; i * particle_radius <= SCR_WIDTH; i += 2) {
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
    // }
}
void Simulation::sample_at_point(vec3 pos) {
    sample_point = pos;
    print_debug_info();

}
void Simulation::print_debug_info() {
    println("-----------------Debug-----------------");

    //Avg Error
    //println("Avg. Error: " << error_sum / simulation_size);
    println("Avg. Error: " << (error_sum / simulation_size) - target_density);
    //Avg Density
    println("Avg. Density: " << density_sum / simulation_size);

    println("");

    //Sample
    //Count Duplicates
    float duplicates = 0;
    for (int i = 0; i < simulation_size + obstacle_particle_count; ++i) {
        Particle& p = particles[i];
        p.color = base_particle_color;
        if (!p.movable)
            p.color = obstacle_color;
        glm::vec<2, int> centre = get_cell_coord(p.pos);
        for (glm::vec<2, int> offset : cell_offsets) {
            int key = KeyFromHash(hash_cell(centre + offset), simulation_size);
            if (key == -1) continue;
            int start = start_indices[key];
            for (int s = start; s < simulation_size; ++s) {
                if (spatial_lookup[s].second != key) break;
                Particle& p2 = particles[spatial_lookup[s].first];
                if (&p != &p2 && distance(p.pos, p2.pos) < particle_radius/(3*scale)) duplicates++;
            }
        }
    }
    println("Duplicates: " << duplicates / 2);

    float density = 0;
    glm::vec<2, int> centre = get_cell_coord(vec2(sample_point));
    for (glm::vec<2, int> offset : cell_offsets) {
        int key = KeyFromHash(hash_cell(centre + offset), simulation_size + obstacle_particle_count);
        if (key == -1) continue;
        int start = start_indices[key];
        for (int s = start; s < simulation_size + obstacle_particle_count; ++s) {
            if (spatial_lookup[s].second != key) break;
            Particle& p2 = particles[spatial_lookup[s].first];
            p2.color = highlighted_particle_color;
            density += sample_density_contribution(p2);
        }
    }
    println("Sample Density: " << density);

    println("---------------------------------------");
}
