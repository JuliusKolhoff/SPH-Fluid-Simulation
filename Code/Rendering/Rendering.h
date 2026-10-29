#pragma once
#include <filesystem>
#include <glad/glad.h>
#include <iostream>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shaders/Shaders.h"
#include "../Simulation/Compute/ComputeShader.h"
#include "../Simulation/Compute/GpuBuffer.h"
#include "Camera.h"
#include "../Simulation/SimParams.h"
#include "../Simulation/GridInfo.h"
#include "../Simulation/Simulation.h"
#include "../Simulation/Compute/Ubo.h"


#define println(x) std::cout << x << std::endl

static unsigned int SCR_WIDTH = 1800;
static unsigned int SCR_HEIGHT = 1000;

struct GridParamsGPU {
	glm::vec4  origin_cell;       // xyz origin, w cellSize
	glm::ivec4 dims_count;        // xyz dims,   w numCells
	uint32_t count;
	uint32_t _pad[3];       // pad to 48 bytes
};
enum Bind : GLuint {
	BIND_POS, BIND_COLOR, BIND_VEL, BIND_MASS, BIND_DENSITY, BIND_PRESSURE, BIND_MOVABLE,
	BIND_CELL_ID, BIND_RANK, BIND_CELL_COUNT, BIND_CELL_START, BIND_SORTED
};

class Rendering
{
private:
	void framebuffer_size_callback(GLFWwindow* window, int width, int height);
	void processInput(GLFWwindow* window,float deltaTime);
public: 
	void mouse_callback(GLFWwindow* window, double xpos, double ypos);
	void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
	void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
	int createWindow(int width, int height, GLFWwindow*& window);
	int start(GLFWwindow* window);
};