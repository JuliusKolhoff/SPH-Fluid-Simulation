#include "Rendering.h"

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

float mouse_xpos = 0.0f;
float mouse_ypos = 0.0f;

std::vector<float> vertices;
std::vector<float> primitive_vertices;

bool run_simulation = false;

//Camera
bool firstMouse = true;
float camera_pitch, camera_yaw;
float lastX = SCR_WIDTH/2, lastY = SCR_HEIGHT/2;
Camera camera;

int Rendering::start(GLFWwindow* window) {
    glfwMakeContextCurrent(window);
	gladLoadGL();

	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

    //REndering settings----
    //glEnable(GL_DEPTH_TEST);
    if (Simulation::get_instance().render_complex) {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);                    // additive glow(soft particles)
    }
    else {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);     // standard 
        glEnable(GL_DEPTH_TEST);
    }
    glEnable(GL_BLEND);
    glEnable(GL_MULTISAMPLE);

    //----
	Simulation::get_instance().create_particles();

    //3d

    glm::mat4 projection;
    projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
    

    glm::mat4 model = glm::identity<mat4>();
    model = glm::scale(model, vec3(Simulation::GRID_SCALE));

    Shader primitive_shader("VertexShader.vert", "PrimitiveShader.frag");
    primitive_shader.Use();
    primitive_shader.setVec2("u_resolution", (float)SCR_WIDTH, (float)SCR_HEIGHT);
    VertexArrayObject primitive_vao;
    BufferObject primitive_vbo(GL_ARRAY_BUFFER, primitive_vertices);

    GLsizei stride = 3 * sizeof(GLfloat);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);


    Shader shader("VertexShader.vert","FragmentShader.frag","GeometryShader.geom");
    shader.Use();
    shader.setVec2("u_resolution", (float)SCR_WIDTH, (float)SCR_HEIGHT);
    shader.setFloat("u_radius", Simulation::get_instance().particle_radius);
    shader.setBool("u_render_complex", Simulation::get_instance().render_complex);
    shader.setFloat("u_radius_increase", Simulation::get_instance().radius_increase);
    shader.setFloat("u_pixels_per_meter", Simulation::GRID_SCALE);
    shader.setMat4("model",model);
    shader.setMat4("view",camera.GetViewMatrix());
    shader.setMat4("projection",projection);

    VertexArrayObject vao;
    
    Simulation::get_instance().update_vertex_data(&vertices);

    BufferObject vbo(GL_ARRAY_BUFFER, vertices);

    //GLsizei stride = sizeof(Particle);
    stride = 6 * sizeof(GLfloat);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3*sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    
    float timer = 120;

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        if (timer == 0) {
		    std::cout << "FPS: " << 1.0f / deltaTime << " Particle Count: "  << Simulation::get_instance().simulation_size + Simulation::get_instance().obstacle_particle_count << std::endl;
            timer = 120;
        }
        timer--;

        // input
        processInput(window,deltaTime);
        shader.setVec2("u_mousePos", mouse_xpos, mouse_ypos);

        //update view and cameraPOs uniforms
        shader.setMat4("view", camera.GetViewMatrix());
        shader.setVec3("u_camera_up", camera.Up);
        shader.setVec3("u_camera_right", camera.Right);
        shader.setVec3("u_camera_position", camera.Position);
        
        
        //Simulation Step
        if (run_simulation)
            Simulation::get_instance().advance(deltaTime, window);

        //Clear Screen
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /*----------------------------------------------------------Draw Patricles--------------------------------------------------------------------------*/
        //Update Vertex Data
        Simulation::get_instance().update_vertex_data(&vertices);
        
            vbo.Bind();
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_DYNAMIC_DRAW);

            shader.Use();
            vao.Bind();

            if (Simulation::get_instance().draw_obstacles) {
                glDrawArrays(GL_POINTS, 0, Simulation::get_instance().simulation_size + Simulation::get_instance().obstacle_particle_count);
            }
            else {
                glDrawArrays(GL_POINTS, 0, Simulation::get_instance().simulation_size);
            }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    delete &Simulation::get_instance();

    glfwDestroyWindow(window);

    glfwTerminate();
    return 0;
}
float cooldown,cooldown2;
void Rendering::processInput(GLFWwindow* window,float deltaTime)
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
    if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS &&  cooldown <= 0) {
        cooldown = 30;
        run_simulation = !run_simulation;
    };

    //DEBUG:
    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS && cooldown <= 0) {
        cooldown = 30;
        Simulation::get_instance().draw_obstacles = !Simulation::get_instance().draw_obstacles;
    }
        
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
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        double x, y;
        glfwGetCursorPos(window, &x, &y);
        y = SCR_HEIGHT - y;
        Simulation::get_instance().sample_at_point(vec3(x, y, 0.0f));
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
    {
        double x, y;
        glfwGetCursorPos(window, &x, &y);
        y = SCR_HEIGHT - y;
        Simulation::get_instance().force_point = vec3(x, y,0);
        Simulation::get_instance().right_mouse_down = true;
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
    {
        Simulation::get_instance().right_mouse_down = false;
    }
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

