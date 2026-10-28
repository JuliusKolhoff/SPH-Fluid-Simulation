#include "Rendering.h"

struct Particle { glm::vec4 pos; glm::vec4 vel; };

const GLuint N = 100000;
std::vector<Particle> data(N);

int Rendering::start(GLFWwindow* window) {
    glfwMakeContextCurrent(window);
    //glfwSwapInterval(0);
	gladLoadGL();

	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);     // standard 
    //glEnable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glEnable(GL_MULTISAMPLE);


    //COMPUTE SHADEr
    srand(time(0));
    for (auto& p : data) {
        p.pos = glm::vec4(((float)rand()/RAND_MAX) * 2 - 1, ((float)rand()/RAND_MAX) * 2, ((float)rand()/RAND_MAX) * 2 - 1, 1);
        p.vel = glm::vec4(0);
    }

    GpuBuffer<Particle> ssbo = GpuBuffer<Particle>(data.data(), N, GL_DYNAMIC_DRAW);
    ssbo.BindBase(0);

    ComputeShader cs = ComputeShader("simulation.comp");

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, ssbo.ID);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)0);

    Shader shader("VertexShader.vert", "FragmentShader.frag");
    
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

        glClearColor(0.05f, 0.05f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        cs.use();
        cs.setFloat("dt", deltaTime);
        cs.setVec3("gravity", 0, -9.8f, 0);
        cs.setUInt("count", N);
        glDispatchCompute((N + 255) / 256, 1, 1);   // ceil(N / local_size_x)

        glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);

        shader.Use();
        glBindVertexArray(vao);
        glDrawArrays(GL_POINTS, 0, N);

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

