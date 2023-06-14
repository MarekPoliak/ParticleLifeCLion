#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include "shader.h"
#include <glm/glm.hpp>
#include <math.h>
#include <glm/gtc/type_ptr.hpp>

void processInput(GLFWwindow *window);

const unsigned int PARTICLE_COUNT = 5000;
const unsigned int COLOR_COUNT = 6;
const glm::vec3 COLORS[COLOR_COUNT] = {
        glm::vec3(1.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 1.0f),
        glm::vec3(1.0f, 1.0f, 0.0f),
        glm::vec3(1.0f, 0.0f, 1.0f),
        glm::vec3(0.0f, 1.0f, 1.0f)
};
float colorMatrix[COLOR_COUNT][COLOR_COUNT];
float colorDropOffIn[COLOR_COUNT];
float colorDropOffOut[COLOR_COUNT];

int main() {
    srand(time(0));
    for (int i = 0; i < COLOR_COUNT; ++i) {
        colorDropOffIn[i] = (rand() % 10 + 1) / 1000.0f;
        colorDropOffOut[i] = (rand() % 10 + 1) / 100.0f;
        for (int j = 0; j < COLOR_COUNT; ++j) {
            colorMatrix[i][j] = (rand() % 21 - 10) / 10.0f;
        }
    }

    if (!glfwInit()) {
        return -1;
    }

    GLFWwindow *window = glfwCreateWindow(1920, 1080, "Hello Triangle", NULL, NULL);
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
    Shader ourShader("shaders/vertex.glsl", "shaders/frag.glsl", "shaders/compute.glsl");

    float vertices[PARTICLE_COUNT * 2];
    float vectors[PARTICLE_COUNT * 2];
    int colorIndices[PARTICLE_COUNT];
    for (int i = 0; i < PARTICLE_COUNT * 2; i = i + 2) {
        vertices[i] = (rand() % 10000) / 5000.0f - 1.0f;
        vertices[i + 1] = (rand() % 10000) / 5000.0f - 1.0f;
        vectors[i] = 0.0f;
        vectors[i + 1] = 0.0f;
        colorIndices[i / 2] = rand() % COLOR_COUNT;
    }

    unsigned int vertexBuffer;
    unsigned int vertexArray;
    glGenVertexArrays(1, &vertexArray);
    glGenBuffers(1, &vertexBuffer);
    glBindVertexArray(vertexArray);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0);

    GLuint vao, positions_vbo, vectors_vbo;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &positions_vbo);
    glGenBuffers(1, &vectors_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, positions_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * PARTICLE_COUNT * 2, &vertices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, vectors_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * PARTICLE_COUNT * 2, &vectors[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (char *) 0 + 0 * sizeof(GLfloat));
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, positions_vbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, vectors_vbo);

    GLuint colorVao, colorVbo;
    glGenVertexArrays(1, &colorVao);
    glBindVertexArray(colorVao);
    glGenBuffers(1, &colorVbo);
    glBindBuffer(GL_ARRAY_BUFFER, colorVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(int) * PARTICLE_COUNT, &colorIndices[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 1, GL_INT, GL_FALSE, 1 * sizeof(GLint), (char *) 0 + 0 * sizeof(GLint));
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, colorVbo);

    float output_buffer[PARTICLE_COUNT * 4];
    GLuint SSBO;
    glGenBuffers(1, &SSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, PARTICLE_COUNT * 4 * sizeof(float), (GLvoid *) output_buffer,
                 GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, SSBO);

    GLint location = glGetUniformLocation(ourShader.COMPUTE_ID, "random");
    ourShader.useVerFrag();
    GLint colorCountLocation = glGetUniformLocation(ourShader.ID, "colorCount");
    glUniform1i(colorCountLocation, COLOR_COUNT);

    GLint colorsLocation = glGetUniformLocation(ourShader.ID, "colors");
    glUniform3fv(colorsLocation, COLOR_COUNT, glm::value_ptr(COLORS[0]));

    GLint colorIndicesLoc = glGetUniformLocation(ourShader.ID, "colorIndices");
    glUniform1iv(colorIndicesLoc, PARTICLE_COUNT, colorIndices);

    glEnable(GL_PROGRAM_POINT_SIZE);
    double lastTime = glfwGetTime();
    int nbFrames = 0;
    while (!glfwWindowShouldClose(window)) {
        processInput(window);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ourShader.useCompute();
        glUniform2f(location, (rand() % 11 - 5.0f) / 100000.0f, (rand() % 11 - 5.0f) / 100000.0f);
        //COMPUTE SHATER BUFFER INPUT
        glBindBuffer(GL_ARRAY_BUFFER, positions_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * PARTICLE_COUNT * 2, &vertices[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, vectors_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * PARTICLE_COUNT * 2, &vectors[0], GL_STATIC_DRAW);

        //COMPUTE SHADER USE
        glDispatchCompute(PARTICLE_COUNT, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);

        //COMPUTE SHADER READ
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO);
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, PARTICLE_COUNT * 4 * sizeof(float), (GLvoid *) output_buffer);
        for (int i = 0; i < PARTICLE_COUNT * 2; i = i + 2) {
            vertices[i] = output_buffer[i * 2];
            vertices[i + 1] = output_buffer[i * 2 + 1];
            vectors[i] = output_buffer[i * 2 + 2];
            vectors[i + 1] = output_buffer[i * 2 + 3];
        }

        ourShader.useVerFrag();

        //PARTICLE DRAW
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glBindVertexArray(vertexArray);
        glDrawArrays(GL_POINTS, 0, PARTICLE_COUNT);

        glfwSwapBuffers(window);
        glfwPollEvents();

        //FPS
        double currentTime = glfwGetTime();
        nbFrames++;
        if (currentTime - lastTime >= 1.0) { // If last prinf() was more than 1 sec ago
            setbuf(stdout, 0);
            std::cout << double(nbFrames) << " FPS" << std::endl;
            nbFrames = 0;
            lastTime += 1.0;
        }
    }
    glfwTerminate();

    return 0;
}

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}
