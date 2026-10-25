#pragma once

#define _USE_MATH_DEFINES
#include <math.h>
#include <algorithm>

const float PI = 3.14159265f;
class KernelManager {
	
public:

	float h = 0, poly6_coef = 0, spiky_grad_coef = 0;
	KernelManager(float r) {
		const float PI = 3.14159265f;
		float h2 = r * r, h3 = h2 * r, h6 = h3 * h3, h9 = h6 * h3;
		h = r;
		poly6_coef = 315.0f / (64.0f * PI * h9);
		spiky_grad_coef = -45.0f / (PI * h6);
	}

	float W_density(float dist, float radius);
	float W1_pressure(float dist, float radius);
	float W1_viscosity(float dist, float radius);

private:
	float smoothing_kernel(float dist, float radius);

	float smoothing_kernel_derivative(float dist, float radius);

	float spiky_kernel(float dist, float radius);

	float spiky_kernel_derivative(float dist, float radius);
};
