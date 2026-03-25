#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>

#include "shader.h"
#include "simulation_calculations.h"
#include "linked_list.h"

// Procedure definitions
void render(GLFWwindow *window);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);

Node* objectList;
Node* wallList;

// Constants
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// Timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;


int main() {
	// Initialise libraries create a window variable
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

	// Enable depth and transparency in OpenGL
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);  

	// Create a shader program
	Shader shaderProgram = initialiseShader("src/shader_vertex.txt", "src/shader_fragment.txt");

	Object stockObject = initialiseObject((char*)"BALL", (vec3){0.0f, 0.0f, 0.0f}, 50.0f, 0.0f, glm_rad(0.0f), 10.0f);

	objectList = initialiseLinkedList(initialiseObject((char*)"BALL", (vec3){400.0f, 300.0f, 0.0f}, 50.0f, 100.0f, glm_rad(70.0f), 10.0f));
	// Object newObj = initialiseObject((char*)"BALL", (vec3){1.0f, 1.0f, 1.0f}, 0.5f, 10.0f, glm_rad(130.0f), 10.0f);
	// appendLinkedList(objectList, newObj);
	// appendLinkedList(objectList, initialiseObject((char*)"BALL", (vec3){100.0f, 100.0f, 0.0f}, 50.0f, 10.0f, glm_rad(130.0f), 10.0f));
	// appendLinkedList(objectList, initialiseObject((char*)"BALL", (vec3){600.0f, 200.0f, 0.0f}, 100.0f, 10.0f, glm_rad(130.0f), 10.0f));
	// appendLinkedList(objectList, initialiseObject((char*)"BALL", (vec3){1000.0f, 1000.0f, 0.0f}, 200.0f, 10.0f, glm_rad(130.0f), 10.0f));

	// printLinkedList(objectList);

	// Linked list holding all of the walls, all currently identical so need to set up the positions and bearings of each
	wallList = initialiseLinkedList(initialiseObject((char*)"WALL", (vec3){0.0f, SCR_HEIGHT, 0.0f}, 0.0f, 0.0f, glm_rad(90.0f), 0.0f)); // top
	appendLinkedList(wallList, initialiseObject((char*)"WALL", (vec3){0.0f, 0.0f, 0.0f}, 0.0f, 0.0f, glm_rad(90.0f), 0.0f)); // bottom
	appendLinkedList(wallList, initialiseObject((char*)"WALL", (vec3){0.0f, 0.0f, 0.0f}, 0.0f, 0.0f, glm_rad(0.0f), 0.0f)); // left
	appendLinkedList(wallList, initialiseObject((char*)"WALL", (vec3){SCR_WIDTH, 0.0f, 0.0f}, 0.0f, 0.0f, glm_rad(0.0f), 0.0f));	// right


	// Shape properties
	int size = 360; // current size with the colour stuff -> may reduce in the future
    float* vertices = (float*)malloc(sizeof(float) * size * 3);
	createObjectVertices(&stockObject, vertices, size);

	// Convert shape properties into a form that is readable by OpenGL
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, size * sizeof(float) * 3, vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);


	// Render loop
	while (!glfwWindowShouldClose(window)) {
		float currentFrame = (float)glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

        // Input
		processInput(window);

        // Rendering
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Activate shader
		use(shaderProgram);

		// Transformations
		mat4 view;
		glm_mat4_identity(view);
		mat4 projection;
		glm_mat4_identity(projection);

		glm_translate(view, (vec3){0.0f, 0.0f, -3.0f});
		int width, height;
		glfwGetWindowSize(window, &width, &height);
		glm_ortho(0.0f, width, 0.0f, height, 0.1f, 100.0f, projection);

		setMat4(shaderProgram, "view", view);
		setMat4(shaderProgram, "projection", projection);

		// Render shape
		glBindVertexArray(VAO);

		Node* traverseObjectList = objectList;

		while (traverseObjectList != NULL) {
			// Check for collisions with walls
			Node* traverseWallList = wallList;
			while (traverseWallList!= NULL) {
				if (checkCollision(&traverseObjectList->obj, &traverseWallList->obj) == true) {
					calcCollision(&traverseObjectList->obj, &traverseWallList->obj, 1);
				} traverseWallList = traverseWallList->next;
			}

			// Check for collisions with other objects
			Node* traverseObjectList2 = objectList;
			while (traverseObjectList2!= NULL) {
				if (traverseObjectList->obj.position[0] == traverseObjectList2->obj.position[0] && traverseObjectList->obj.position[1] == traverseObjectList2->obj.position[1]) {
					traverseObjectList2 = traverseObjectList2->next;
					continue;
				}
				
				if (checkCollision(&traverseObjectList->obj, &traverseObjectList2->obj) == true) {
					calcCollision(&traverseObjectList->obj, &traverseObjectList2->obj, 1);
				} traverseObjectList2 = traverseObjectList2->next;
			}

			moveObject(&traverseObjectList->obj, deltaTime);
			
			mat4 model;
			glm_mat4_identity(model);
			glm_translate(model, traverseObjectList->obj.position);
			float scaleFactor = traverseObjectList->obj.radius / stockObject.radius;
			glm_scale(model, (vec3){scaleFactor, scaleFactor, scaleFactor});

			setMat4(shaderProgram, "model", model);

			glDrawArrays(GL_TRIANGLE_FAN, 0, size);

			traverseObjectList = traverseObjectList->next;	
		}

        // Check and call events, Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	freeLinkedList(objectList);
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	// glDeleteBuffers(1, &EBO);
	glDeleteProgram(shaderProgram.ID);
	
	glfwTerminate();
	return 0;
}

void processInput(GLFWwindow *window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		appendLinkedList(objectList, initialiseObject((char*)"BALL", (vec3){200.0f, 100.0f, 0}, 10.0f, 10.0f, glm_rad(130.0f), 10.0f));
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
	glViewport(0, 0, width, height);
}