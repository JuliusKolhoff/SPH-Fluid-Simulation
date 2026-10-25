#pragma once

#define _USE_MATH_DEFINES
#include <math.h>
#include <algorithm>

const float PI = 3.14159265f;

class KernelManager {
	
public:
	float W_density(float dist, float radius);
	float W1_pressure(float dist, float radius);
	float W1_viscosity(float dist, float radius);

private:
	float smoothing_kernel(float dist, float radius);

	float smoothing_kernel_derivative(float dist, float radius);

	
	float spiky_kernel(float dist, float radius);

	float spiky_kernel_derivative(float dist, float radius);
};
