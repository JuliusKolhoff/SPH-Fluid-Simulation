#include "Rendering.h"

int Rendering::start(GLFWwindow* window) {
    glfwMakeContextCurrent(window);
    glfwSwapInterval(0);
	gladLoadGL();

	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

    //REndering settings----
    if (Simulation::get_instance().render_complex) {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);                    // additive glow(soft particles)
    }
    else {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);     // standard 
        glEnable(GL_DEPTH_TEST);
    }
    glEnable(GL_BLEND);
    glEnable(GL_MULTISAMPLE);

    
    float deltaTime, lastFrame = static_cast<float>(glfwGetTime());
    int timer = 0;
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        
        if (timer > 120) {
            timer -= 120;
            std::cout << "FPS: " << 1.0f / deltaTime <<  std::endl;
        }
        timer++;

        processInput(window, deltaTime);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);

    glfwTerminate();
    return 0;
}
void Rendering::processInput(GLFWwindow* window,float deltaTime)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}
int Rendering::createWindow(int width,int height, GLFWwindow*& window) {
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    window = glfwCreateWindow(width, height, "SPH FLuid Simulation", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    //glfwSetWindowAttrib(window, GLFW_RESIZABLE, GL_FALSE);

    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *mode = glfwGetVideoMode(monitor);

    glfwSetWindowPos(window, mode->width / 2 - width / 2, mode->height / 2 - height / 2);

    return 0;
}
void Rendering::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void Rendering::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
}

void Rendering::mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{

}
void Rendering::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
}

