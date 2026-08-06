#include <iostream>
#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include"Rendering.h"

#define check(x) if(x != 0){return -1;}

void mouse_callback(GLFWwindow * window, double xpos, double ypos);
void scroll_callback(GLFWwindow * window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow * window, int button, int action, int mods);


Rendering renderer;

int main() {
	renderer = Rendering();
	GLFWwindow* window;
	glfwWindowHint(GLFW_SAMPLES, 4);
	check(renderer.createWindow(SCR_WIDTH, SCR_HEIGHT, window));
	
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	renderer.start(window);
}
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	renderer.mouse_button_callback(window, button, action, mods);
}
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	renderer.mouse_callback(window, xpos, ypos);
}
void scroll_callback(GLFWwindow* window, double xpos, double ypos) {
	renderer.scroll_callback(window, xpos, ypos);
}
