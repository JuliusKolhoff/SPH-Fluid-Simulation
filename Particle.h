#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>


class Particle {
public:
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec3 velocity;
	float mass = 1.0f;
	float density = 1.0f;
	float pressure = 0.0f;
	bool movable = true;
	Particle() {
		pos = { 0,0,0 };
		color = { 0,0,0};
		velocity = { 0.0f,0.0f,0.0f };
	}
	Particle(float x, float y, float z, float r, float g, float b) {
		pos = { x,y,z };
		color = { r,g,b };
		velocity = {0.0f,0.0f,0.0f};
	}
};
