layout(std140, binding = 1) uniform SimParams {
    float h, h2;
    float target_density, stiffness;
    float viscosity, gravity;
    float collision_damping, particle_radius;
    float poly6_coef, spiky_grad_coef;
    float count;
    vec4  bounds_min;
    vec4  bounds_max;
} sim;

layout(std140, binding = 2) uniform FrameParams {
    float dt;
    float step_size;
} frame;

#define H        (sim.h)
#define H2       (sim.h2)
#define DT       (frame.dt)
#ifndef COUNT
#define COUNT (sim.count)
#endif

float poly6(float r2) { 
    float v = H2 - r2;
    return v > 0.0 ? sim.poly6_coef * v * v * v : 0.0;
}
float spikyGrad(float r) {
    float v = H - r;
    return v > 0.0 ? sim.spiky_grad_coef * v * v : 0.0;
}