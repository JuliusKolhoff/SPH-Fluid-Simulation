#include "Rendering.h"

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

float mouse_xpos = 0.0f;
float mouse_ypos = 0.0f;

std::vector<float> vertices;
//{
//    //POS             //Color
//    0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f
//};

void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

int Rendering::start(GLFWwindow* window) {
    glfwMakeContextCurrent(window);
	gladLoadGL();

	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    glEnable(GL_DEPTH_TEST);

	Simulation::get_instance().create_particles();

    Shader shader("VertexShader.vert","FragmentShader.frag","GeometryShader.geom");
    shader.Use();
    shader.setVec2("u_resolution", (float)SCR_WIDTH, (float)SCR_HEIGHT);
    shader.setFloat("u_radius", Simulation::get_instance().particle_radius);

    VertexArrayObject vao;
   
    BufferObject vbo(GL_ARRAY_BUFFER, Simulation::get_instance().particles);
    //BufferObject vbo(GL_ARRAY_BUFFER, vertices);

    GLsizei stride = sizeof(Particle);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3*sizeof(GLfloat)));
    glEnableVertexAttribArray(1);



    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

		std::cout << "FPS: " << 1.0f / deltaTime << std::endl;
        
        //Update Vertex Data
        /*vertices.clear();
        for (const auto& particle : Simulation::get_instance().particles) {
            vertices.push_back(particle.pos.x);
            vertices.push_back(particle.pos.y);
            vertices.push_back(particle.pos.z);
            vertices.push_back(particle.color.r);
            vertices.push_back(particle.color.g);
            vertices.push_back(particle.color.b);
        }*/

        Simulation::get_instance().step(deltaTime);
		vbo.Bind();
		glBufferData(GL_ARRAY_BUFFER, Simulation::get_instance().particles.size() * sizeof(Particle), Simulation::get_instance().particles.data(), GL_DYNAMIC_DRAW);

        // input
        processInput(window);
        shader.setVec2("u_mousePos", mouse_xpos, mouse_ypos);
        // render
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.Use();
        vao.Bind();

        

        glDrawArrays(GL_POINTS, 0, Simulation::get_instance().simulation_size);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    

    glfwDestroyWindow(window);

    glfwTerminate();
    return 0;
}
void Rendering::render(Particle* particles, int size)
{

}
void Rendering::processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {};
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {};
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {};
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {};
}
int Rendering::createWindow(int width,int height, GLFWwindow*& window) {
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    window = glfwCreateWindow(width, height, "Hello World", NULL, NULL);
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

void Rendering::mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    mouse_xpos = static_cast<float>(xposIn);
    mouse_ypos = static_cast<float>(yposIn);
    
   /* float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);*/
}
void Rendering::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    //camera.ProcessMouseScroll(static_cast<float>(yoffset));
}
