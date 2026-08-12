#include "Rendering.h"

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

float mouse_xpos = 0.0f;
float mouse_ypos = 0.0f;

std::vector<float> vertices;
std::vector<float> primitive_vertices;
//{
//    //POS             //Color
//    0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f
//};
bool run_simulation = false;

glm::mat4 view;

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
    }
    glEnable(GL_BLEND);

    glEnable(GL_MULTISAMPLE);

    //----
	Simulation::get_instance().create_particles();

    //3d

    view = glm::mat4(1.0f);
    // note that we're translating the scene in the reverse direction of where we want to move
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -1.0f));

    glm::mat4 projection;
    projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

    glm::mat4 model;
    model = glm::mat4(1.0f);




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
    shader.setFloat("u_pixels_per_meter", Simulation::PIXELS_PER_METER);
    shader.setMat4("model",model);
    shader.setMat4("view",view);
    shader.setMat4("projection",projection);

    VertexArrayObject vao;
    
    //BufferObject vbo(GL_ARRAY_BUFFER, Simulation::get_instance().particles);
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

    
    

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

		//std::cout << "FPS: " << 1.0f / deltaTime << std::endl;
        shader.setMat4("view", view);
        //Simulation Step
        if (run_simulation) {
            Simulation::get_instance().step(deltaTime,window);

        }

        //Clear Screen
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /*----------------------------------------------------------Draw Patricles--------------------------------------------------------------------------*/
        //Update Vertex Data
        Simulation::get_instance().update_vertex_data(&vertices);
        for (int i = 0; i < 20; i++) {
            for (size_t i = 0; i < Simulation::get_instance().simulation_size + Simulation::get_instance().obstacle_particle_count; i++)
            {
                vertices[2 + i * 3] += 1.0f;
            }
            vbo.Bind();
            //glBufferData(GL_ARRAY_BUFFER, Simulation::get_instance().particles.size() * sizeof(Particle), Simulation::get_instance().particles.data(), GL_DYNAMIC_DRAW);
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_DYNAMIC_DRAW);

            // input
            processInput(window);
            shader.setVec2("u_mousePos", mouse_xpos, mouse_ypos);

            shader.Use();
            vao.Bind();
            bool draw_obstacles = true;
            if (draw_obstacles) {
                glDrawArrays(GL_POINTS, 0, Simulation::get_instance().simulation_size + Simulation::get_instance().obstacle_particle_count);
            }
            else {
                glDrawArrays(GL_POINTS, 0, Simulation::get_instance().simulation_size);
            }
        }
		

        /*----------------------------------------------------------Draw Primitives--------------------------------------------------------------------------*/
        /*primitive_vertices.clear();
        
        for (int i = 0; i < Simulation::get_instance().simulation_size; i++) {
            Particle& particle = Simulation::get_instance().particles[i];
            primitive_vertices.push_back(particle.pos.x);
            primitive_vertices.push_back(particle.pos.y);
            primitive_vertices.push_back(particle.pos.z);
            primitive_vertices.push_back(particle.pos.x + particle.velocity.x);
            primitive_vertices.push_back(particle.pos.y + particle.velocity.y);
            primitive_vertices.push_back(particle.pos.z + particle.velocity.z);

        }


        primitive_vbo.Bind();
        glBufferData(GL_ARRAY_BUFFER, primitive_vertices.size() * sizeof(GLfloat), primitive_vertices.data(), GL_DYNAMIC_DRAW);

        primitive_shader.Use();
        primitive_vao.Bind();

        glDrawArrays(GL_LINES, 0, primitive_vertices.size()/3);*/

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    delete &Simulation::get_instance();

    glfwDestroyWindow(window);

    glfwTerminate();
    return 0;
}
float cooldown;
void Rendering::processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    cooldown--;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) { view = glm::translate(view, glm::vec3(0.0f, 0.0f, 0.1f));/*Simulation::get_instance().k += 0.1f;*/};
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) { view = glm::translate(view, glm::vec3(0.0f, 0.0f, -0.1f));/*Simulation::get_instance().k -= 0.1f; */};
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) { view = glm::translate(view, glm::vec3(0.1f, 0.0f, 0.0f));/*Simulation::get_instance().target_density += 0.1; */};
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) { view = glm::translate(view, glm::vec3(-0.1f, 0.0f, 0.0f));/*Simulation::get_instance().target_density -= 0.1;*/ };
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) { view = glm::translate(view, glm::vec3(0.0f, -0.1f, 0.0f));/*Simulation::get_instance().target_density += 0.1; */ };
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) { view = glm::translate(view, glm::vec3(0.0f, 0.1f, 0.0f));/*Simulation::get_instance().target_density -= 0.1;*/ };
    if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS &&  cooldown <= 0) {
        cooldown = 30;
        run_simulation = !run_simulation;
    };
    /*if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        Simulation::get_instance().simulation_size += 10;
        Simulation::get_instance().create_particles();
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        Simulation::get_instance().simulation_size =
            glm::max(10u, Simulation::get_instance().simulation_size - 10);
        Simulation::get_instance().create_particles();
    }*/
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
    
    
    /*float xpos = static_cast<float>(xposIn);
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

