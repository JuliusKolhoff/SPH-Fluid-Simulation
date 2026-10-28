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

#define println(x) std::cout << x << std::endl

static unsigned int SCR_WIDTH = 1800;
static unsigned int SCR_HEIGHT = 1000;

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