#include <iostream>
#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include"Rendering.h"

#define check(x) if(x != 0){return -1;}

void mouse_callback(GLFWwindow * window, double xpos, double ypos);
void scroll_callback(GLFWwindow * window, double xpos, double ypos);

Rendering renderer;

int main() {
	renderer = Rendering();
	GLFWwindow* window;
	check(renderer.createWindow(SCR_WIDTH, SCR_HEIGHT, window));
	
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	renderer.start(window);
}
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	renderer.mouse_callback(window, xpos, ypos);
}
void scroll_callback(GLFWwindow* window, double xpos, double ypos) {
	renderer.scroll_callback(window, xpos, ypos);
}
