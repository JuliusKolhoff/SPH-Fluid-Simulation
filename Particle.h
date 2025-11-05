#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

struct Particle {
	glm::vec3 pos;
	glm::vec3 color;
	Particle(float x, float y, float z, float r, float g, float b) {
		pos = { x,y,z };
		color = { r,g,b };
	}
};