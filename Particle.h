#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

struct Particle {
	glm::vec3 pos;
	glm::vec3 color;
	float mass = 1.0f;
	float density = 1.0f;
	Particle(float x, float y, float z, float r, float g, float b) {
		pos = { x,y,z };
		color = { r,g,b };
	}
};