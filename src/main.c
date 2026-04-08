/* 
	This project is a Physics Simulation which aims to provide a toolkit for an A-Level Physics or Maths student can use 
	to experiment with ideas learnt about in mechanics topics in lesson.
*/

// Graphics Libraries
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

// Nuklear GUI library
#define MAX_VERTEX_BUFFER 512 * 1024
#define MAX_ELEMENT_BUFFER 128 * 1024
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_GLFW_GL3_IMPLEMENTATION
#include <nuklear/nuklear.h>
#include <nuklear/nuklear_glfw_gl3.h>

// Standard C libraries
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <math.h>
#include <string.h>

// Custom header files
#include "shader.h"
#include "simulation_calculations.h"
#include "linked_list.h"

// Procedure definitions
struct nk_image generateTexture(char* image);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window, int key, int scancode, int action, int mods);
void togglePause();
void toggleNavbar();
void toggleTerminal();
void toggleSettings();

// Linked list initialisations
Node* objectList;
Node* wallList;

// Constants
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
const float BUTTON_PADDING = 10.0f;
const float BUTTON_SIZE = 40.0f;
const float navbarWidth = 200.0f;


// Timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Object ID counter
int id = 0;

// Flags
bool pause = false;
bool navbar = false;
bool terminal = false;
bool settings = false;

// Simulation Colour Scheme:
// 4F 6D 7A		0.31, 0.427, 0.478
// C0 D6 DF		0.753, 0.839, 0.875
// DB E9 EE		0.859, 0.914, 0.933
// 4A 6F A5		0.29, 0.435, 0.647
// 16 60 88		0.086, 0.376, 0.533

int main() {
	// Initialise libraries create a window variable
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	#ifdef __APPLE__
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	#endif

	// Creation of a window using GLFW
	struct nk_glfw glfw = {0};
	GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Physics Simulation", NULL, NULL);
	if (window == NULL) {
        printf("Failed to create GLFW window\n");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	// Callbacks provide 'interrupts' for the programs and will handle a task when it happens instead of checking for a condition constantly in the render loop
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetKeyCallback(window, processInput);

	// Initialise GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        printf("Failed to initialise GLAD\n");
		return -1;
	}

	// Create a context, this is required for nuklear UI to be used 
	struct nk_context *ctx = nk_glfw3_init(&glfw, window, NK_GLFW3_INSTALL_CALLBACKS);
	{
		struct nk_font_atlas *atlas;
		nk_glfw3_font_stash_begin(&glfw, &atlas);
		nk_glfw3_font_stash_end(&glfw);
	}
	// Create a style for the buttons: similar to the job css does in web development
	struct nk_style_button style = ctx->style.button;
	style.padding       = nk_vec2(0, 0);
	style.image_padding = nk_vec2(0, 0);
	style.touch_padding = nk_vec2(0, 0);
	style.border        = 0;
	style.rounding      = 0;

	// Enable depth and transparency in OpenGL
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);  
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Create a shader program
	Shader shaderProgram = initialiseShader("src/shader_vertex.txt", "src/shader_fragment.txt");

	// Create a stock object: this object will be used for transformations to all of the other objects but this individual object will never be rendered
	Object stockObject = initialiseObject(-1, (char*)"BALL", (vec3){0.0f, 0.0f, 0.0f}, 50.0f, 0.0f, glm_rad(0.0f), 10.0f);

	// Add objects into the data structure where they are stored
	objectList = initialiseLinkedList(initialiseObject(id, (char*)"BALL", (vec3){200.0f, 300.0f, 0.0f}, 50.0f, 100.0f, glm_rad(90.0f), 10.0f));
	id++;
	// Object newObj = initialiseObject(id, (char*)"BALL", (vec3){600.0f, 300.0f, 0.0f}, 50.0f, 100.0f, glm_rad(270.0f), 10.0f);
	// appendLinkedList(objectList, newObj);
	// id++;
	// appendLinkedList(objectList, initialiseObject((char*)"BALL", (vec3){100.0f, 100.0f, 0.0f}, 50.0f, 10.0f, glm_rad(130.0f), 10.0f));
	// appendLinkedList(objectList, initialiseObject((char*)"BALL", (vec3){600.0f, 200.0f, 0.0f}, 100.0f, 10.0f, glm_rad(130.0f), 10.0f));
	// appendLinkedList(objectList, initialiseObject((char*)"BALL", (vec3){1000.0f, 1000.0f, 0.0f}, 200.0f, 10.0f, glm_rad(130.0f), 10.0f));

	// printLinkedList(objectList);

	// Linked list holding all of the walls, all currently identical so need to set up the positions and bearings of each
	wallList = initialiseLinkedList(initialiseObject(-1, (char*)"WALL", (vec3){0.0f, SCR_HEIGHT, 0.0f}, 0.0f, 0.0f, glm_rad(90.0f), 0.0f)); // top
	appendLinkedList(wallList, initialiseObject(-1, (char*)"WALL", (vec3){0.0f, 0.0f, 0.0f}, 0.0f, 0.0f, glm_rad(90.0f), 0.0f)); // bottom
	appendLinkedList(wallList, initialiseObject(-1, (char*)"WALL", (vec3){0.0f, 0.0f, 0.0f}, 0.0f, 0.0f, glm_rad(0.0f), 0.0f)); // left
	appendLinkedList(wallList, initialiseObject(-1, (char*)"WALL", (vec3){SCR_WIDTH, 0.0f, 0.0f}, 0.0f, 0.0f, glm_rad(0.0f), 0.0f));	// right


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

	// Import the images for each of the UI top buttons into the program
	struct nk_image pauseButtonImage = generateTexture((char*)"img/pause.png");
	struct nk_image settingsButtonImage = generateTexture((char*)"img/settings.png");
	struct nk_image terminalButtonImage = generateTexture((char*)"img/terminal.png");
	struct nk_image navbarButtonImage = generateTexture((char*)"img/navbar.png");

	// Render loop
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		nk_glfw3_new_frame(&glfw);

		// Calculate the time passed since last iteration
		float currentFrame = (float)glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Find new window size, this is needed incase the user resizes the window
		int width, height;
		glfwGetWindowSize(window, &width, &height);

		// GUI 
		// Top buttons
		if (navbar == false) {
			nk_style_push_style_item(ctx, &ctx->style.window.fixed_background, nk_style_item_color(nk_rgba(0,0,0,0)));
			if (nk_begin(ctx, "Button Wrapper", nk_rect(width - (BUTTON_PADDING*4 + BUTTON_SIZE*4), BUTTON_PADDING, BUTTON_SIZE*4 + BUTTON_PADDING*3, BUTTON_SIZE * 1.2), NK_WINDOW_NO_SCROLLBAR)) {
				nk_layout_row_dynamic(ctx, BUTTON_SIZE, 4);
				if (nk_button_image_styled(ctx, &style, pauseButtonImage)) togglePause(); 
				if (nk_button_image_styled(ctx, &style, settingsButtonImage)) toggleSettings();
				if (nk_button_image_styled(ctx, &style, terminalButtonImage)) toggleTerminal();
				if (nk_button_image_styled(ctx, &style, navbarButtonImage)) toggleNavbar();

			} nk_end(ctx);
			nk_style_pop_style_item(ctx);
		} else {
			nk_style_push_style_item(ctx, &ctx->style.window.fixed_background, nk_style_item_color(nk_rgba(0,0,0,0)));
			if (nk_begin(ctx, "Button Wrapper", nk_rect(width - (BUTTON_PADDING*4 + BUTTON_SIZE*4 + navbarWidth), BUTTON_PADDING, BUTTON_SIZE*4 + BUTTON_PADDING*3, BUTTON_SIZE * 1.2), NK_WINDOW_NO_SCROLLBAR)) {
				nk_layout_row_dynamic(ctx, BUTTON_SIZE, 4);
				if (nk_button_image_styled(ctx, &style, pauseButtonImage)) togglePause(); 
				if (nk_button_image_styled(ctx, &style, settingsButtonImage)) toggleSettings();
				if (nk_button_image_styled(ctx, &style, terminalButtonImage)) toggleTerminal();
				if (nk_button_image_styled(ctx, &style, navbarButtonImage)) toggleNavbar();

			} nk_end(ctx);
			nk_style_pop_style_item(ctx);

			// ------                           width-(width * 0.25)
			if (nk_begin(ctx, "Navbar", nk_rect(width-200, 0, width, height), 0)) {
				nk_layout_row_dynamic(ctx, 120, 1);
				nk_label(ctx, "Hello world!", NK_TEXT_LEFT);

				nk_layout_row_static(ctx, 50, 100, 1);
				if (nk_button_label(ctx, "Button"))
					fprintf(stdout, "pressed\n");
			} nk_end(ctx);
		}


	

        // Rendering
		glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Activate shader
		use(shaderProgram);

		// Transformations
		mat4 view;
		glm_mat4_identity(view);
		mat4 projection;
		glm_mat4_identity(projection);

		glm_translate(view, (vec3){0.0f, 0.0f, -3.0f});
		glm_ortho(0.0f, width, 0.0f, height, 0.1f, 100.0f, projection);

		// Updates the boundaries of the simulation to the border of the window: this is incase the window has been resized
		Node* updateWallList = wallList;
		while (updateWallList != NULL) {
			if (updateWallList->obj.position[1] != 0.0f) {
				updateWallList->obj.position[1] = height;
			} else if (updateWallList->obj.position[0] != 0.0f) {
				updateWallList->obj.position[0] = width;
			} updateWallList = updateWallList->next;
		} 

		// Send uniforms to the shader program
		setVec3(shaderProgram, "aColour", (vec3){0.29, 0.435, 0.647});
		setMat4(shaderProgram, "view", view);
		setMat4(shaderProgram, "projection", projection);

		// Render shapes
		glBindVertexArray(VAO);

		// Iterate through each object in the object list
		Node* traverseObjectList = objectList;
		while (traverseObjectList != NULL) {
			// Checks the pause flag, if the simulation is paused, no collision calculations or movement of objects should be completed
			if (pause == false) {	
				// Check for collisions with walls
				Node* traverseWallList = wallList;
				while (traverseWallList!= NULL) {
					if (checkCollision(&traverseObjectList->obj, &traverseWallList->obj) == true) {
						calcCollision(&traverseObjectList->obj, &traverseWallList->obj, 1.0f);
					} traverseWallList = traverseWallList->next;
				} 

				// Check for collisions with other objects
				Node* traverseObjectList2 = objectList;
				while (traverseObjectList2!= NULL) {
					// If the object is checking for collisions against itself, skip this check
					if (traverseObjectList->obj.id == traverseObjectList2->obj.id) {
						traverseObjectList2 = traverseObjectList2->next;
						continue;
					}
					
					if (checkCollision(&traverseObjectList->obj, &traverseObjectList2->obj) == true) {
						calcCollision(&traverseObjectList->obj, &traverseObjectList2->obj, 1.0f);
					} traverseObjectList2 = traverseObjectList2->next;
				} 
				// If the object is out of bounds, delete the object
				if (traverseObjectList->obj.position[2] < 0) {
					deleteNode(objectList, traverseObjectList->obj.id);
				}
				moveObject(&traverseObjectList->obj, deltaTime);
			}
			
			// Update model matrix for current objects new position and size 
			mat4 model;
			glm_mat4_identity(model);
			glm_translate(model, traverseObjectList->obj.position);
			float scaleFactor = traverseObjectList->obj.radius / stockObject.radius;
			glm_scale(model, (vec3){scaleFactor, scaleFactor, scaleFactor});

			// Send as a uniform to the shader program
			setMat4(shaderProgram, "model", model);

			glDrawArrays(GL_TRIANGLE_FAN, 0, size);

			traverseObjectList = traverseObjectList->next;	
		}
		// printLinkedList(objectList);

		nk_glfw3_render(&glfw, NK_ANTI_ALIASING_ON, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);
		glfwSwapBuffers(window);
	}
	freeLinkedList(objectList);
	freeLinkedList(wallList);
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteProgram(shaderProgram.ID);
	
	nk_glfw3_shutdown(&glfw);
	glfwTerminate();
	return 0;
}

// Load the stated image into the program using OpenGL and converting it into a format Nuklear can use
struct nk_image generateTexture(char* image) {
    unsigned int tex_id;
    glGenTextures(1, &tex_id);
    glBindTexture(GL_TEXTURE_2D, tex_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    unsigned char *data = stbi_load(image, &width, &height, &nrChannels, 0);

	if (data) {
    	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	} else {
		printf("ERROR::TEXTURE_GENERATION\n");
	} stbi_image_free(data);

    return nk_image_id((int)tex_id);
}

// Handle user input
void processInput(GLFWwindow *window, int key, int scancode, int action, int mods) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		appendLinkedList(objectList, initialiseObject(id, (char*)"BALL", (vec3){200.0f, 100.0f, 0}, 10.0f, 10.0f, glm_rad(130.0f), 10.0f));
		id++;

	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
		pause = !pause;
}

// Allows the user to resize the winodw
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
	glViewport(0, 0, width, height);
}

void togglePause() {
    pause = !pause;
}
void toggleNavbar() {
	navbar = !navbar;
}
void toggleTerminal() {
	terminal = !terminal;
}
void toggleSettings() {
	settings = !settings;
}