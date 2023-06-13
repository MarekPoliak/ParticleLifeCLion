#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include "shader.h"
#include <chrono>
#include <thread>
#include <math.h>

void processInput(GLFWwindow *window);

const unsigned int PARTICLE_COUNT = 5000;

float magnitude(float a, float b);

int main() {
    // initialize GLFW
    if (!glfwInit()) {
        return -1;
    }

    // create a window pointer
    GLFWwindow *window = glfwCreateWindow(1920, 1080, "Hello Triangle", NULL, NULL);
    // error check
    if (window == NULL) {
        std::cout << "Error. I could not create a window at all!" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // build and compile our shader zprogram
    // ------------------------------------
    Shader ourShader("shaders/vertex.glsl", "shaders/frag.glsl", "shaders/compute.glsl");

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    srand(time(0));
    float vertices[PARTICLE_COUNT * 2];
    float vectors[PARTICLE_COUNT * 2];
    for (int i = 0; i < PARTICLE_COUNT * 2; i = i + 2) {
        vertices[i] = (rand() % 10000) / 5000.0f - 1.0f;
        vertices[i + 1] = (rand() % 10000) / 5000.0f - 1.0f;
        vectors[i] = 0.0f;
        vectors[i + 1] = 0.0f;
    }


    unsigned int vertexBuffer;
    unsigned int vertexArray;
    // buffer
    glGenVertexArrays(1, &vertexArray);
    glGenBuffers(1, &vertexBuffer);

    glBindVertexArray(vertexArray);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *) 0);

    glEnableVertexAttribArray(0);

    // generate positions_vbos and vaos
    GLuint vao, positions_vbo, vectors_vbo;

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &positions_vbo);
    glGenBuffers(1, &vectors_vbo);

    glBindBuffer(GL_ARRAY_BUFFER, positions_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * PARTICLE_COUNT * 2, &vertices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, vectors_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * PARTICLE_COUNT * 2, &vectors[0], GL_STATIC_DRAW);

    // set up generic attrib pointers
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (char *) 0 + 0 * sizeof(GLfloat));

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, positions_vbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, vectors_vbo);

    float output_buffer[PARTICLE_COUNT * 4];
    GLuint SSBO2;
    glGenBuffers(1, &SSBO2);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO2);
    glBufferData(GL_SHADER_STORAGE_BUFFER, PARTICLE_COUNT * 4 * sizeof(float), (GLvoid *) output_buffer,
                 GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, SSBO2);

    GLuint location = glGetUniformLocation(ourShader.COMPUTE_ID, "random");

    double lastTime = glfwGetTime();
    int nbFrames = 0;

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window)) {
        glUniform2f(location, (rand() % 11 - 5.0f) / 100000.0f, (rand() % 11 - 5.0f) / 100000.0f);
        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);


        glBindBuffer(GL_ARRAY_BUFFER, positions_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * PARTICLE_COUNT * 2, &vertices[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, vectors_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * PARTICLE_COUNT * 2, &vectors[0], GL_STATIC_DRAW);

        // render container
        ourShader.use();
        glDispatchCompute(PARTICLE_COUNT, 1, 1);

        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO2);
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, PARTICLE_COUNT * 4 * sizeof(float), (GLvoid *) output_buffer);

        for (int i = 0; i < PARTICLE_COUNT * 2; i = i + 2) {
            vertices[i] = output_buffer[i * 2];
            vertices[i + 1] = output_buffer[i * 2 + 1];
            vectors[i] = output_buffer[i * 2 + 2];
            vectors[i + 1] = output_buffer[i * 2 + 3];
        }

        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindVertexArray(vertexArray);
        glDrawArrays(GL_POINTS, 0, PARTICLE_COUNT);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
        //std::this_thread::sleep_for(std::chrono::milliseconds(5));

        double currentTime = glfwGetTime();
        nbFrames++;
        if (currentTime - lastTime >= 1.0) { // If last prinf() was more than 1 sec ago
            // printf and reset timer
            setbuf(stdout, 0);
            std::cout << double(nbFrames) << " ms/frame\n" << std::endl;
            nbFrames = 0;
            lastTime += 1.0;
        }
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

float magnitude(float a, float b) {
    return sqrt(a * a + b * b);
}
