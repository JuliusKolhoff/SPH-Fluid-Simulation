#include "Kernels.h";

float KernelManager::W_density(float dist, float radius)
{
    return smoothing_kernel(dist, radius);
}
float KernelManager::W1_pressure(float dist, float radius)
{
    return spiky_kernel_derivative(dist, radius);
}
float KernelManager::W1_viscosity(float dist, float radius)
{
    //return smoothing_kernel_derivative(dist,radius);
    return spiky_kernel_derivative(dist, radius);
}

float KernelManager::smoothing_kernel(float dist, float radius) {
    float v = h * h - dist * dist;
    return v > 0 ? poly6_coef * v * v * v : 0.0f;
}

float KernelManager::smoothing_kernel_derivative(float dist, float radius) {
    if (dist > radius) return 0.0f;
    // d/dr Poly6 (3D): -945 / (32 π h^9) * r * (h² - r²)²
    float f = radius * radius - dist * dist;
    float scale = -945.0f / (32.0f * PI * pow(radius, 9));
    return scale * dist * f * f;
}

float KernelManager::spiky_kernel(float dist, float radius) {
    if (dist > radius) return 0.0f;
    // Spiky (3D): 15 / (π h^6) * (h - r)³
    float h_r = radius - dist;
    float volume = PI * pow(radius, 6) / 15.0f;
    return (h_r * h_r * h_r) / volume;
}

float KernelManager::spiky_kernel_derivative(float dist, float radius) {
    float v = h - dist;
    return v > 0 ? spiky_grad_coef * v * v : 0.0f;
}


