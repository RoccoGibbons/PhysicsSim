#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>

// #include <shader.h>

// Procedure definitions
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);
int length(char* myString);
void addChar(char *s, char c);

// Constants
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;


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

    // Reads file
    FILE* vShaderFile = fopen("src/shader.vs", "r");
    if (vShaderFile == NULL) {
        printf("ERROR::SHADER::VERTEX::FILE_NOT_READ\n");
    }
	FILE* fShaderFile = fopen("src/shader.fs", "r");
    if (fShaderFile == NULL) {
        printf("ERROR::SHADER::FRAGMENT::FILE_NOT_READ\n");
    }

    // Sets a pointer to the end of the file
    // Finds the position of the end of the file and saves that value as the length of file
    // Sets the pointer back the the beginning of the file 
    fseek(vShaderFile, 0, SEEK_END);
    unsigned int vFileSize = ftell(vShaderFile);
    rewind(vShaderFile);

	fseek(fShaderFile, 0, SEEK_END);
    unsigned int fFileSize = ftell(fShaderFile);
    rewind(fShaderFile);

    // Sets a buffer for each line of the file and allocates memory for the entire file to be saved
    char buffer[128];
	char* vertexCode = (char*)malloc(vFileSize * sizeof(char));
    // Makes sure the allocated memory starts off as blank
    strcpy(vertexCode, "");

    // Concatenates buffer onto vertexCode for each line
    while(fgets(buffer, 128, vShaderFile) != NULL) {
		strcat(vertexCode, buffer);
	}

	// Repeated for fragment shader
	char* fragmentCode = (char*)malloc(fFileSize * sizeof(char));
    strcpy(fragmentCode, "");

    while(fgets(buffer, 128, fShaderFile) != NULL) {
		strcat(fragmentCode, buffer);
	}

	fclose(vShaderFile);
	fclose(fShaderFile);

	// Build shader programs
	unsigned int vertexShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	// glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glShaderSource(vertexShader, 1, &vertexCode, NULL);
	glCompileShader(vertexShader);

	{	
		int success;
		char infoLog[512];
		glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
		if (!success) {
			glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
			printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED %s /n", infoLog);
		}
	}

	unsigned int fragmentShader;
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	// glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glShaderSource(fragmentShader, 1, &fragmentCode, NULL);
	glCompileShader(fragmentShader);
	
	{
		int success;
		char infoLog[512];
		glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
		if (!success) {
			glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
			printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED %s /n", infoLog);
		}
	}

	// Link shader programs
	unsigned int shaderProgram;
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	{
		int success;
		char infoLog[512];
		glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
		if (!success) {
			glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
			printf("ERROR::SHADER::PROGRAM::LINK_FAILED %s /n", infoLog);
		}
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

    free(vertexCode);
    vertexCode = NULL;	
	free(fragmentCode);
	fragmentCode = NULL;

	// Shader shaderProgram = initialise_shader("shader.vs", "shader.fs");

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
		glUseProgram(shaderProgram);
		// use(shaderProgram);

		// Colourful stuff
		float timeValue = glfwGetTime();
		float greenValue = (sin(timeValue) / 2.0f) + 0.5f;
		int vertexColourLocation = glGetUniformLocation(shaderProgram, "ourColour");
		glUseProgram(shaderProgram);
		// use(shaderProgram);
		glUniform4f(vertexColourLocation, 0.0f, greenValue, 0.0f, 1.0f);


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
	glDeleteProgram(shaderProgram);
	
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

int length(char* myString) {
    int counter = 0;
    while (myString[counter] != '\n') {
        counter += 1;
    }
    return counter;
}

void addChar(char *s, char c) {
    while (*s++);
	*(s - 1) = c;
  	*s = '\0';
}