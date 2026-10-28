#include "Rendering.h"

using glm::vec3,glm::mat4;

bool firstMouse = true;
float camera_pitch, camera_yaw;
float lastX = SCR_WIDTH / 2, lastY = SCR_HEIGHT / 2;
Camera camera;

int Rendering::start(GLFWwindow* window) {
    glfwMakeContextCurrent(window);
    //glfwSwapInterval(0);
	gladLoadGL();

	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);     // standard 
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glEnable(GL_MULTISAMPLE);

    glm::mat4 projection;
    projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);

    float scale = 1.0f;
    glm::mat4 model = glm::identity<mat4>();
    model = glm::scale(model, vec3(scale));

    SimParams& params = Simulation::get_instance().params;

    //COMPUTE SHADEr
    srand(time(0));
    for (int i = 0; i < params.n; i++) {
        Simulation::get_instance().pos[i] = glm::vec4(((float)rand() / RAND_MAX) * 10, ((float)rand() / RAND_MAX) * 10, ((float)rand() / RAND_MAX) * -10, 1);
        Simulation::get_instance().vel[i] = glm::vec4(0);
        Simulation::get_instance().color[i] = glm::vec4(1.0f);
    }
    //SET Count for helper
    GLuint ubo;
    glGenBuffers(1, &ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(GLuint), &params.n, GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);


    GpuBuffer posBuffer = GpuBuffer(params.n*sizeof(glm::vec4), Simulation::get_instance().pos, GL_DYNAMIC_DRAW);
    posBuffer.BindBase(BIND_POS);
    GpuBuffer colorBuffer = GpuBuffer(params.n * sizeof(glm::vec4), Simulation::get_instance().color, GL_DYNAMIC_DRAW);
    colorBuffer.BindBase(BIND_COLOR);
    GpuBuffer velBuffer = GpuBuffer(params.n * sizeof(glm::vec4), Simulation::get_instance().vel, GL_DYNAMIC_DRAW);
    velBuffer.BindBase(BIND_VEL);
    GpuBuffer massBuffer = GpuBuffer(params.n * sizeof(float), Simulation::get_instance().mass, GL_DYNAMIC_DRAW);
    massBuffer.BindBase(BIND_MASS);
    GpuBuffer densityBuffer = GpuBuffer(params.n * sizeof(glm::vec2), Simulation::get_instance().density, GL_DYNAMIC_DRAW);
    densityBuffer.BindBase(BIND_DENSITY);
    GpuBuffer pressureBuffer = GpuBuffer(params.n * sizeof(float), Simulation::get_instance().pressure, GL_DYNAMIC_DRAW);
    pressureBuffer.BindBase(BIND_PRESSURE);
    GpuBuffer movableBuffer = GpuBuffer(params.n * sizeof(unsigned int), Simulation::get_instance().movable, GL_DYNAMIC_DRAW);
    movableBuffer.BindBase(BIND_MOVABLE);

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, posBuffer.id);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, colorBuffer.id);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);

    glm::vec3 bmin = glm::min(glm::vec3(0), params.bounds);   // (0, 0, -15)
    glm::vec3 bmax = glm::max(glm::vec3(0), params.bounds);   // (20, 15, 0)
    GridInfo grid = makeGrid(bmin, bmax, params.smoothing_radius);

    GridParamsGPU gp;
    gp.origin_cell = glm::vec4(grid.origin, grid.cellSize);
    gp.dims_count = glm::ivec4(grid.dims, (int)grid.numCells);
    gp.count = params.n;

    glGenBuffers(1, &ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(gp), &gp, GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);

    GpuBuffer cellIDBuffer = GpuBuffer(params.n * sizeof(unsigned int));
    cellIDBuffer.BindBase(BIND_CELL_ID);
    GpuBuffer rankBuffer = GpuBuffer(params.n * sizeof(unsigned int));
    rankBuffer.BindBase(BIND_RANK);
    GpuBuffer cellCountBuffer = GpuBuffer(grid.numCells * sizeof(unsigned int));
    cellCountBuffer.BindBase(BIND_CELL_COUNT);
    GpuBuffer cellStartBuffer = GpuBuffer((grid.numCells + 1) * sizeof(unsigned int));
    cellStartBuffer.BindBase(BIND_CELL_START);
    GpuBuffer sortedIdsBuffer = GpuBuffer(params.n * sizeof(unsigned int));
    sortedIdsBuffer.BindBase(BIND_SORTED);

    Shader shader("VertexShader.vert", "FragmentShader.frag", "GeometryShader.geom");
    shader.Use();
    shader.setFloat("u_radius", 0.16f);
    shader.setMat4("model", model);
    shader.setMat4("view", camera.GetViewMatrix());
    shader.setMat4("projection", projection);
    
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

        Simulation::get_instance().step(deltaTime);

        shader.Use();
        shader.setMat4("view", camera.GetViewMatrix());
        shader.setVec3("u_camera_up", camera.Up);
        shader.setVec3("u_camera_right", camera.Right);
        shader.setVec3("u_camera_position", camera.Position);
        glBindVertexArray(vao);
        glDrawArrays(GL_POINTS, 0, params.n);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);

    glfwTerminate();
    return 0;
}
float cooldown;
void Rendering::processInput(GLFWwindow* window, float deltaTime)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    cooldown--;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) { camera.ProcessKeyboard(Camera_Movement::FORWARD, deltaTime); };
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) { camera.ProcessKeyboard(Camera_Movement::BACKWARD, deltaTime); };
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) { camera.ProcessKeyboard(Camera_Movement::LEFT, deltaTime); };
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) { camera.ProcessKeyboard(Camera_Movement::RIGHT, deltaTime); };
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) { camera.ProcessKeyboard(Camera_Movement::UP, deltaTime); };
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) { camera.ProcessKeyboard(Camera_Movement::DOWN, deltaTime); };

    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS && cooldown <= 0) {
        camera.ProcessKeyboard(Camera_Movement::SPRINT, deltaTime);
        cooldown = 30;
    }
}
int Rendering::createWindow(int width,int height, GLFWwindow*& window) {
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    glfwWindowHint(GLFW_SAMPLES, 4);

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
    float xpos = static_cast<float>(xposIn);
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

    camera.ProcessMouseMovement(xoffset, yoffset);

}
void Rendering::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

