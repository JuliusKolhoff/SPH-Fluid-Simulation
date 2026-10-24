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
    if (dist > radius) return 0.0f;
    // Poly6 (3D): 315 / (64 π h^9) * (h² - r²)³
    float volume = 64.0f * M_PI * pow(radius, 9) / 315.0f;
    float value = std::max(0.0f, radius * radius - dist * dist);
    return value * value * value / volume;
}

float KernelManager::smoothing_kernel_derivative(float dist, float radius) {
    if (dist > radius) return 0.0f;
    // d/dr Poly6 (3D): -945 / (32 π h^9) * r * (h² - r²)²
    float f = radius * radius - dist * dist;
    float scale = -945.0f / (32.0f * M_PI * pow(radius, 9));
    return scale * dist * f * f;
}

float KernelManager::spiky_kernel(float dist, float radius) {
    if (dist > radius) return 0.0f;
    // Spiky (3D): 15 / (π h^6) * (h - r)³
    float h_r = radius - dist;
    float volume = M_PI * pow(radius, 6) / 15.0f;
    return (h_r * h_r * h_r) / volume;
}

float KernelManager::spiky_kernel_derivative(float dist, float radius) {
    if (dist > radius) return 0.0f;
    // d/dr Spiky (3D): -45 / (π h^6) * (h - r)²
    float h_r = radius - dist;
    float scale = 45.0f / (M_PI * pow(radius, 6));
    return -scale * h_r * h_r;
}


