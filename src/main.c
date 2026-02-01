#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#include "shader.h"

// Procedure definitions
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);

// Constants
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// Shaders

// const char *vertexShaderSource = "#version 330 core\n"
// "layout (location = 0) in vec3 aPos;\n"
// "layout (location = 1) in vec3 aColour;\n"
// "out vec3 ourColour;\n"
// "void main() {\n"
// "gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
// "ourColour = aColour;\n"
// "}\0";

// const char *fragmentShaderSource = "#version 330 core\n"
// "out vec4 FragColour;\n"
// "in vec3 ourColour;\n"
// "void main () {\n"
// "FragColour = vec4(ourColour, 1.0);\n"
// "}\0";

int main() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Physics Simulation", NULL, NULL);
	if (window == NULL) {
        printf("Failed to create GLFW window\n");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        printf("Failed to initialise GLAD\n");
		return -1;
	}

	// Build shader programs
	// unsigned int vertexShader;
	// vertexShader = glCreateShader(GL_VERTEX_SHADER);
	// glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	// glCompileShader(vertexShader);

	// {	
	// 	int success;
	// 	char infoLog[512];
	// 	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	// 	if (!success) {
	// 		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
	// 		printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED %s /n", infoLog);
	// 	}
	// }

	// unsigned int fragmentShader;
	// fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	// glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	// glCompileShader(fragmentShader);
	
	// {
	// 	int success;
	// 	char infoLog[512];
	// 	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	// 	if (!success) {
	// 		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
	// 		printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED %s /n", infoLog);
	// 	}
	// }

	// // Link shader programs
	// unsigned int shaderProgram;
	// shaderProgram = glCreateProgram();
	// glAttachShader(shaderProgram, vertexShader);
	// glAttachShader(shaderProgram, fragmentShader);
	// glLinkProgram(shaderProgram);

	// {
	// 	int success;
	// 	char infoLog[512];
	// 	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	// 	if (!success) {
	// 		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
	// 		printf("ERROR::SHADER::PROGRAM::LINK_FAILED %s /n", infoLog);
	// 	}
	// }

	// glDeleteShader(vertexShader);
	// glDeleteShader(fragmentShader);

	Shader shaderProgram = initialise_shader("shader.vs", "shader.fs");
	

    float vertices[] = {
        0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
		-0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f
    	// 0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,   
    	// -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,   
    	// 0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f    
    };

	unsigned int indices[] = {
		0, 1, 3,
		1, 2, 3
	}; 

    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	while (!glfwWindowShouldClose(window)) {
        // Input
		processInput(window);

        // Rendering
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// Activate shader
		// glUseProgram(shaderProgram);
		use(shaderProgram);

		// Colourful stuff
		float timeValue = glfwGetTime();
		float greenValue = (sin(timeValue) / 2.0f) + 0.5f;
		int vertexColourLocation = glGetUniformLocation(shaderProgram.ID, "ourColour");
		use(shaderProgram);
		glUniform4f(vertexColourLocation, 0.0f, greenValue, 0.0f, 1.0f);

		// Render triangle
		glBindVertexArray(VAO);
		// glDrawArrays(GL_TRIANGLES, 0, 3);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // Check and call events, Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
	glDeleteProgram(shaderProgram.ID);
	
	glfwTerminate();
	return 0;
}

void processInput(GLFWwindow *window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
	glViewport(0, 0, width, height);
}