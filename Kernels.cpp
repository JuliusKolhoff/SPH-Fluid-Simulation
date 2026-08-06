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

float KernelManager::smoothing_kernel(float  dist, float radius) {
    radius /= 100;
    dist /= 100;
    if (dist > radius) return 0;
    float volume = M_PI * pow(radius, 8) / 4;
    float value = std::max(0.0f, radius * radius - dist * dist);
    return (value * value * value / volume);
}
float KernelManager::smoothing_kernel_derivative(float  dist, float radius) {
    radius /= 100;
    dist /= 100;
    if (dist > radius) return 0;
    float f = radius * radius - dist * dist;
    float scale = -24 / (M_PI * pow(radius, 8));
    return scale * dist * f * f;
}

float KernelManager::spiky_kernel(float dist, float radius) {
    radius /= 100;
    dist /= 100;
    if (dist > radius) return 0.0f;
    float h_r = radius - dist;
    float volume = M_PI * pow(radius, 5) / 10.0f;  // from 2-D normalisation above
    return ((h_r * h_r * h_r) / volume);
}
float KernelManager::spiky_kernel_derivative(float dist, float radius) {
    radius /= 100;
    dist /= 100;
    if (dist > radius) return 0.0f;
    float h_r = radius - dist;
    float scale = 30.0f / (M_PI * pow(radius, 5));
    return -scale * h_r * h_r;
}


