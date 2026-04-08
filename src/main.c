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
#define NK_MAX_NUMBER_BUFFER 1024
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
void setValue(char* value, char* identifier, int width, int height, Object* obj);
void updateTerminal(struct nk_context *ctx, float currentFrame);

// Linked list initialisations
Node* objectList;
Node* wallList;

// Constants
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
const float BUTTON_PADDING = 10.0f;
const float BUTTON_SIZE = 40.0f;
const float navbarWidth = 200.0f;
const float terminalHeight = 100.0f;

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

// Simulation Variables
float accelerationDueToGravity = -100.0f;
float scale = 1.0f;
float timeScale = 1.0f;
float coefficientOfRestitutionObject = 1.0f;
float coefficientOfRestitutionWall = 1.0f;

// Terminal tracker
int currentTerminalObject = 0;
enum terminalObjectProperty {position, speed, bearing};
const char* terminalObjectProperties[] = {"Position", "Speed", "Bearing"};
int currentTerminalObjectProperty = 0;
float printRate = 0.5;

// Terminal variables
#define MAX_ENTRIES 64
char entries[MAX_ENTRIES][64];
int entryCount = 0;
float lastUpdate = 0.0f;

// Settings Page
int page = 0;

// Simulation Colour Scheme:
// 4F 6D 7A		0.31, 0.427, 0.478		79, 109, 122
// C0 D6 DF		0.753, 0.839, 0.875		192, 214, 223
// DB E9 EE		0.859, 0.914, 0.933		219, 233, 238
// 4A 6F A5		0.29, 0.435, 0.647		74, 111, 165
// 16 60 88		0.086, 0.376, 0.533		22, 96, 136

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

	// Initialise GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        printf("Failed to initialise GLAD\n");
		return -1;
	}

	// Create a context, this is required for nuklear UI to be used 
	struct nk_context *ctx = nk_glfw3_init(&glfw, window, NK_GLFW3_INSTALL_CALLBACKS);

	// Callbacks provide 'interrupts' for the programs and will handle a task when it happens instead of checking for a condition constantly in the render loop
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetKeyCallback(window, processInput);


	// Set fonts for UI
	struct nk_font_atlas *atlas;
	nk_glfw3_font_stash_begin(&glfw, &atlas);
	struct nk_font *font_small = nk_font_atlas_add_from_file(atlas, "resources/font/century.TTF", 14, 0);
	struct nk_font *font_medium = nk_font_atlas_add_from_file(atlas, "resources/font/century.TTF", 20, 0);
	struct nk_font *font_large = nk_font_atlas_add_from_file(atlas, "resources/font/century.TTF", 36, 0);
	nk_glfw3_font_stash_end(&glfw);
	nk_style_set_font(ctx, &font_medium->handle);                  // then set font
	
	// Create a style for the buttons: similar to the job css does in web development
	struct nk_style_button buttonStyle = ctx->style.button;
	buttonStyle.padding       = nk_vec2(0, 0);
	buttonStyle.image_padding = nk_vec2(0, 0);
	buttonStyle.touch_padding = nk_vec2(0, 0);
	buttonStyle.border        = 0;
	buttonStyle.rounding      = 0;

	// Using another button as a background to different elements (e.g. title)
	struct nk_style_button textBackgroundStyle = ctx->style.button;
	textBackgroundStyle.normal = nk_style_item_color(nk_rgb(219, 233, 238));
	textBackgroundStyle.hover = nk_style_item_color(nk_rgb(219, 233, 238));
	textBackgroundStyle.active = nk_style_item_color(nk_rgb(219, 233, 238));
	textBackgroundStyle.text_normal = nk_rgb(22, 96, 136);
	textBackgroundStyle.text_hover = nk_rgb(22, 96, 136);
	textBackgroundStyle.text_active = nk_rgb(22, 96, 136);
	textBackgroundStyle.border = 0;
	textBackgroundStyle.rounding = 0;


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
	appendLinkedList(wallList, initialiseObject(-2, (char*)"WALL", (vec3){0.0f, 0.0f, 0.0f}, 0.0f, 0.0f, glm_rad(90.0f), 0.0f)); // bottom
	appendLinkedList(wallList, initialiseObject(-3, (char*)"WALL", (vec3){0.0f, 0.0f, 0.0f}, 0.0f, 0.0f, glm_rad(0.0f), 0.0f)); // left
	appendLinkedList(wallList, initialiseObject(-4, (char*)"WALL", (vec3){SCR_WIDTH, 0.0f, 0.0f}, 0.0f, 0.0f, glm_rad(0.0f), 0.0f));	// right


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
	struct nk_image pauseButtonImage = generateTexture((char*)"resources/img/pause.png");
	struct nk_image settingsButtonImage = generateTexture((char*)"resources/img/settings.png");
	struct nk_image terminalButtonImage = generateTexture((char*)"resources/img/terminal.png");
	struct nk_image navbarButtonImage = generateTexture((char*)"resources/img/navbar.png");

	const char* text = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Quisque porttitor nisl ligula. Vestibulum id nisl et orci aliquam posuere. Nulla gravida ultricies suscipit. Suspendisse est purus, dignissim eget cursus id, semper quis nulla. Aenean quis purus ac eros dictum imperdiet vel quis ante. Curabitur luctus consectetur nibh vitae vestibulum. Fusce quis dolor justo. Curabitur euismod sodales justo sit amet efficitur. Sed posuere, nisl iaculis tincidunt vestibulum, est nisi sagittis orci, in cursus lacus turpis sed dui. ";
	// Buffers to handle input in UI
	char* accbuf = (char*)malloc(sizeof(char) * 256);
	int accbuf_len = 0;
	char scaleBuf[32];
	char timeBuf[32];
	char eoBuf[32];
	char ewBuf[32];
	char printBuf[32];
	char* objbuf = (char*)malloc(sizeof(char) * 256);
	int objbuf_len = 0;
	char* posxBuf = (char*)malloc(sizeof(char) * 256);
	int posxBuf_len = 0;
	char* posyBuf = (char*)malloc(sizeof(char) * 256);
	int posyBuf_len = 0;
	char* radBuf = (char*)malloc(sizeof(char) * 256);
	int radBuf_len = 0;
	char* speedBuf = (char*)malloc(sizeof(char) * 256);
	int speedBuf_len = 0;
	char* bearBuf = (char*)malloc(sizeof(char) * 256);
	int bearBuf_len = 0;
	char* massBuf = (char*)malloc(sizeof(char) * 256);
	int massBuf_len = 0;

	// Render loop
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		nk_glfw3_new_frame(&glfw);

		// Calculate the time passed since last iteration
		float currentFrame = (float)glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		deltaTime *= timeScale;

		// Find new window size, this is needed incase the user resizes the window
		int width, height;
		glfwGetWindowSize(window, &width, &height);

		// GUI 
		if (navbar == false) {
			// Top buttons
			nk_style_push_style_item(ctx, &ctx->style.window.fixed_background, nk_style_item_color(nk_rgba(0,0,0,0)));
			if (nk_begin(ctx, "Button Wrapper", nk_rect(width - (BUTTON_PADDING*4 + BUTTON_SIZE*4), BUTTON_PADDING, BUTTON_SIZE*4 + BUTTON_PADDING*3, BUTTON_SIZE * 1.2), NK_WINDOW_NO_SCROLLBAR)) {
				nk_layout_row_dynamic(ctx, BUTTON_SIZE, 4);
				if (nk_button_image_styled(ctx, &buttonStyle, pauseButtonImage)) togglePause(); 
				if (nk_button_image_styled(ctx, &buttonStyle, settingsButtonImage)) toggleSettings();
				if (nk_button_image_styled(ctx, &buttonStyle, terminalButtonImage)) toggleTerminal();
				if (nk_button_image_styled(ctx, &buttonStyle, navbarButtonImage)) toggleNavbar();

			} nk_end(ctx);
			nk_style_pop_style_item(ctx);
		} else {
			// Top buttons
			nk_style_push_style_item(ctx, &ctx->style.window.fixed_background, nk_style_item_color(nk_rgba(0,0,0,0)));
			if (nk_begin(ctx, "Button Wrapper", nk_rect(width - (BUTTON_PADDING*4 + BUTTON_SIZE*4 + navbarWidth), BUTTON_PADDING, BUTTON_SIZE*4 + BUTTON_PADDING*3, BUTTON_SIZE * 1.2), NK_WINDOW_NO_SCROLLBAR)) {
				nk_layout_row_dynamic(ctx, BUTTON_SIZE, 4);
				if (nk_button_image_styled(ctx, &buttonStyle, pauseButtonImage)) togglePause(); 
				if (nk_button_image_styled(ctx, &buttonStyle, settingsButtonImage)) toggleSettings();
				if (nk_button_image_styled(ctx, &buttonStyle, terminalButtonImage)) toggleTerminal();
				if (nk_button_image_styled(ctx, &buttonStyle, navbarButtonImage)) toggleNavbar();

			} nk_end(ctx);
			nk_style_pop_style_item(ctx);

			// Navbar
			nk_style_push_style_item(ctx, &ctx->style.window.fixed_background, nk_style_item_color(nk_rgba(192, 214, 223, 255)));
			nk_style_push_color(ctx, &ctx->style.window.border_color, nk_rgb(79, 109, 122));
			nk_style_push_float(ctx, &ctx->style.window.border, 2.0f);
			if (nk_begin(ctx, "Navbar", nk_rect(width-navbarWidth, 0, navbarWidth, height), NK_WINDOW_BORDER)) {
				// Navbar Title
				nk_style_set_font(ctx, &font_large->handle);
				nk_layout_row_dynamic(ctx, 0, 1);
				nk_button_label_styled(ctx, &textBackgroundStyle, "Navbar");

				// Subheading 1: Simulation Variables
				nk_style_set_font(ctx, &font_medium->handle);
				nk_layout_row_dynamic(ctx, 0, 1);
				nk_button_label_styled(ctx, &textBackgroundStyle, "Simulation Variables");

				// Value 1: Acceleration due to gravity
				nk_style_set_font(ctx, &font_small->handle);
				nk_layout_row_dynamic(ctx, 0, 1);
				nk_button_label_styled(ctx, &textBackgroundStyle, "Acceleration due to Gravity:");
				nk_layout_row_template_begin(ctx, 35);       
				nk_layout_row_template_push_dynamic(ctx);       
				nk_layout_row_template_push_dynamic(ctx);   
				nk_layout_row_template_end(ctx);

				nk_edit_string(ctx, NK_EDIT_SIMPLE, accbuf, &accbuf_len, 255, nk_filter_default); 
				if (nk_button_label_styled(ctx, &textBackgroundStyle, "Set")) setValue(accbuf, (char*)"acc", 0, 0, 0);

				// Value 2: Scale
				nk_layout_row_dynamic(ctx, 0, 2);
				nk_button_label_styled(ctx, &textBackgroundStyle, "Scale:");
				snprintf(scaleBuf, sizeof(scaleBuf), "%.1f", scale);
				nk_button_label_styled(ctx, &textBackgroundStyle, scaleBuf);
				nk_layout_row_dynamic(ctx, 0, 1);
				nk_slider_float(ctx, 0.0f, &scale, 10.0f, 0.1f);

				// Value 3: Time Scale
				nk_layout_row_dynamic(ctx, 0, 2);
				nk_button_label_styled(ctx, &textBackgroundStyle, "Time Scale:");
				snprintf(timeBuf, sizeof(timeBuf), "%.2f", timeScale);
				nk_button_label_styled(ctx, &textBackgroundStyle, timeBuf);
				nk_layout_row_dynamic(ctx, 0, 1);
				nk_slider_float(ctx, 0.01f, &timeScale, 10.0f, 0.01f);

				// Value 4: Coefficient of Restitution - Object
				nk_layout_row_template_begin(ctx, 0);
				nk_layout_row_template_push_dynamic(ctx);
				nk_layout_row_template_push_static(ctx, 50);
				nk_layout_row_template_end(ctx);
				nk_button_label_styled(ctx, &textBackgroundStyle, "Elasticity - Object:");
				snprintf(eoBuf, sizeof(eoBuf), "%.2f", coefficientOfRestitutionObject);
				nk_button_label_styled(ctx, &textBackgroundStyle, eoBuf);
				nk_layout_row_dynamic(ctx, 0, 1);
				nk_slider_float(ctx, 0.0f, &coefficientOfRestitutionObject, 1.0f, 0.01f);

				// Value 5: Coefficient of Restitution - Wall
				nk_layout_row_template_begin(ctx, 0);
				nk_layout_row_template_push_dynamic(ctx);
				nk_layout_row_template_push_static(ctx, 50);
				nk_layout_row_template_end(ctx);
				nk_button_label_styled(ctx, &textBackgroundStyle, "Elasticity - Wall:");
				snprintf(ewBuf, sizeof(ewBuf), "%.2f", coefficientOfRestitutionWall);
				nk_button_label_styled(ctx, &textBackgroundStyle, ewBuf);
				nk_layout_row_dynamic(ctx, 0, 1);
				nk_slider_float(ctx, 0.0f, &coefficientOfRestitutionWall, 1.0f, 0.01f);
				
				nk_layout_row_static(ctx, 10, 0, 1);
				nk_label(ctx, "", NK_TEXT_CENTERED);
				
				// Subheading 2: Terminal Settings
				nk_style_set_font(ctx, &font_medium->handle);
				nk_layout_row_dynamic(ctx, 0, 1);
				nk_button_label_styled(ctx, &textBackgroundStyle, "Terminal Settings");

				// Value 1: Object to track
				nk_style_set_font(ctx, &font_small->handle);
				nk_layout_row_dynamic(ctx, 0, 1);
				nk_button_label_styled(ctx, &textBackgroundStyle, "Object ID:");
				nk_layout_row_template_begin(ctx, 35);       
				nk_layout_row_template_push_dynamic(ctx);       
				nk_layout_row_template_push_dynamic(ctx);   
				nk_layout_row_template_end(ctx);

				nk_edit_string(ctx, NK_EDIT_SIMPLE, objbuf, &objbuf_len, 255, nk_filter_default); 
				if (nk_button_label_styled(ctx, &textBackgroundStyle, "Set")) setValue(objbuf, (char*)"id", 0, 0, 0);

				// Value 2: Property to track
				nk_layout_row_dynamic(ctx, 0, 1);
				nk_button_label_styled(ctx, &textBackgroundStyle, "Object Property:");
				nk_layout_row_dynamic(ctx, 0, 1);
				currentTerminalObjectProperty = nk_combo(ctx, terminalObjectProperties, 3, currentTerminalObjectProperty, 20, nk_vec2(150, 50));

				// Value 3: Rate of updating
				nk_layout_row_template_begin(ctx, 0);
				nk_layout_row_template_push_dynamic(ctx);
				nk_layout_row_template_push_static(ctx, 50);
				nk_layout_row_template_end(ctx);
				nk_button_label_styled(ctx, &textBackgroundStyle, "Print Rate:");
				snprintf(printBuf, sizeof(printBuf), "%.1f", printRate);
				nk_button_label_styled(ctx, &textBackgroundStyle, printBuf);
				nk_layout_row_dynamic(ctx, 0, 1);
				nk_slider_float(ctx, 0.0f, &printRate, 10.0f, 0.1f);
				
				nk_layout_row_static(ctx, 10, 0, 1);
				nk_label(ctx, "", NK_TEXT_CENTERED);

				// Subheading 3: Objects
				nk_style_set_font(ctx, &font_medium->handle);
				nk_layout_row_dynamic(ctx, 0, 1);
				nk_button_label_styled(ctx, &textBackgroundStyle, "Objects:");

				// Iterate through every object in the simulation
				Node* traverseObjectListUI = objectList;
				while (traverseObjectListUI != NULL) { 
					nk_style_set_font(ctx, &font_medium->handle);
					// Object ID/ Subheading for particular object
					char id[5]; 
					nk_itoa(id, traverseObjectListUI->obj.id);
					char title[50];
					strcpy(title, "Object ");
					strcat(title, id);
					nk_layout_row_dynamic(ctx, 0, 1);
					nk_button_label_styled(ctx, &textBackgroundStyle, title);
					
					nk_style_set_font(ctx, &font_medium->handle);

					nk_layout_row_dynamic(ctx, 0, 1);
					nk_button_label_styled(ctx, &textBackgroundStyle, "Position:");

					nk_layout_row_template_begin(ctx, 35);       
					nk_layout_row_template_push_dynamic(ctx);       
					nk_layout_row_template_push_dynamic(ctx);   
					nk_layout_row_template_end(ctx);
					nk_edit_string(ctx, NK_EDIT_SIMPLE, posxBuf, &posxBuf_len, 255, nk_filter_default); 
					if (nk_button_label_styled(ctx, &textBackgroundStyle, "Set X")) setValue(posxBuf, (char*)"posx", width, height, &traverseObjectListUI->obj);

					nk_layout_row_template_begin(ctx, 35);       
					nk_layout_row_template_push_dynamic(ctx);       
					nk_layout_row_template_push_dynamic(ctx);   
					nk_layout_row_template_end(ctx);
					nk_edit_string(ctx, NK_EDIT_SIMPLE, posyBuf, &posyBuf_len, 255, nk_filter_default); 
					if (nk_button_label_styled(ctx, &textBackgroundStyle, "Set Y")) setValue(posyBuf, (char*)"posy", width, height, &traverseObjectListUI->obj);

					nk_layout_row_dynamic(ctx, 0, 1);
					nk_button_label_styled(ctx, &textBackgroundStyle, "Radius:");
					nk_layout_row_template_begin(ctx, 35);       
					nk_layout_row_template_push_dynamic(ctx);       
					nk_layout_row_template_push_dynamic(ctx);   
					nk_layout_row_template_end(ctx);
					nk_edit_string(ctx, NK_EDIT_SIMPLE, radBuf, &radBuf_len, 255, nk_filter_default); 
					if (nk_button_label_styled(ctx, &textBackgroundStyle, "Set Radius")) setValue(radBuf, (char*)"rad", width, height, &traverseObjectListUI->obj);

					nk_layout_row_dynamic(ctx, 0, 1);
					nk_button_label_styled(ctx, &textBackgroundStyle, "Speed:");
					nk_layout_row_template_begin(ctx, 35);       
					nk_layout_row_template_push_dynamic(ctx);       
					nk_layout_row_template_push_dynamic(ctx);   
					nk_layout_row_template_end(ctx);
					nk_edit_string(ctx, NK_EDIT_SIMPLE, speedBuf, &speedBuf_len, 255, nk_filter_default); 
					if (nk_button_label_styled(ctx, &textBackgroundStyle, "Set Speed")) setValue(speedBuf, (char*)"spe", width, height, &traverseObjectListUI->obj);

					nk_layout_row_dynamic(ctx, 0, 1);
					nk_button_label_styled(ctx, &textBackgroundStyle, "Bearing:");
					nk_layout_row_template_begin(ctx, 35);       
					nk_layout_row_template_push_dynamic(ctx);       
					nk_layout_row_template_push_dynamic(ctx);   
					nk_layout_row_template_end(ctx);
					nk_edit_string(ctx, NK_EDIT_SIMPLE, bearBuf, &bearBuf_len, 255, nk_filter_default); 
					if (nk_button_label_styled(ctx, &textBackgroundStyle, "Set Bearing")) setValue(bearBuf, (char*)"bea", width, height, &traverseObjectListUI->obj);

					nk_layout_row_dynamic(ctx, 0, 1);
					nk_button_label_styled(ctx, &textBackgroundStyle, "Mass:");
					nk_layout_row_template_begin(ctx, 35);       
					nk_layout_row_template_push_dynamic(ctx);       
					nk_layout_row_template_push_dynamic(ctx);   
					nk_layout_row_template_end(ctx);
					nk_edit_string(ctx, NK_EDIT_SIMPLE, massBuf, &massBuf_len, 255, nk_filter_default); 
					if (nk_button_label_styled(ctx, &textBackgroundStyle, "Set Mass")) setValue(radBuf, (char*)"mass", width, height, &traverseObjectListUI->obj);

					nk_layout_row_static(ctx, 5, 0, 1);
					nk_label(ctx, "", NK_TEXT_CENTERED);
					traverseObjectListUI = traverseObjectListUI->next;
				}

			} nk_end(ctx);
			nk_style_pop_float(ctx);
			nk_style_pop_color(ctx);
			nk_style_pop_style_item(ctx);
		}
		// Terminal
		if (terminal) { 
			// Update the terminal display
			if (printRate > 0) {
				updateTerminal(ctx, currentFrame);
			}

			// Terminal UI
			nk_style_push_style_item(ctx, &ctx->style.window.fixed_background, nk_style_item_color(nk_rgba(192, 214, 223, 255)));
			nk_style_push_color(ctx, &ctx->style.window.border_color, nk_rgb(79, 109, 122));
			nk_style_push_float(ctx, &ctx->style.window.border, 2.0f);
			if (nk_begin(ctx, "Terminal", nk_rect(0, height - terminalHeight, width, terminalHeight), NK_WINDOW_BORDER)) {
				for (int i = entryCount - 1; i >= 0; i--) {
					nk_layout_row_dynamic(ctx, 0, 1);
					nk_button_label_styled(ctx, &textBackgroundStyle, entries[i]);
				}
			} nk_end(ctx);
			nk_style_pop_float(ctx);
			nk_style_pop_color(ctx);
			nk_style_pop_style_item(ctx);
		}

		
		// Settings
		if (settings) {
			nk_style_push_style_item(ctx, &ctx->style.window.fixed_background, nk_style_item_color(nk_rgba(192, 214, 223, 255)));
			nk_style_push_color(ctx, &ctx->style.window.border_color, nk_rgb(79, 109, 122));
			nk_style_push_float(ctx, &ctx->style.window.border, 2.0f);
			if (nk_begin(ctx, "Settings", nk_rect(width * 0.1, height * 0.1, width * 0.8, height * 0.8), NK_WINDOW_BORDER)) {
				nk_style_set_font(ctx, &font_large->handle);
				nk_layout_row_dynamic(ctx, 50, 3);
				if (nk_button_label_styled(ctx, &textBackgroundStyle, "Key Binds")) page = 0;
				if (nk_button_label_styled(ctx, &textBackgroundStyle, "Presets")) page = 1;
				if (nk_button_label_styled(ctx, &textBackgroundStyle, "Options")) page = 2;

				if (page == 0) {
					nk_style_set_font(ctx, &font_medium->handle);
					nk_layout_row_dynamic(ctx, 0, 2);
					nk_button_label_styled(ctx, &textBackgroundStyle, "Summon New Object");
					nk_button_label_styled(ctx, &textBackgroundStyle, "Space Bar");

					nk_layout_row_dynamic(ctx, 0, 2);
					nk_button_label_styled(ctx, &textBackgroundStyle, "Pause Simulation");
					nk_button_label_styled(ctx, &textBackgroundStyle, "P");

					nk_layout_row_dynamic(ctx, 0, 2);
					nk_button_label_styled(ctx, &textBackgroundStyle, "Toggle Navbar");
					nk_button_label_styled(ctx, &textBackgroundStyle, "M");

					nk_layout_row_dynamic(ctx, 0, 2);
					nk_button_label_styled(ctx, &textBackgroundStyle, "Toggle Terminal");
					nk_button_label_styled(ctx, &textBackgroundStyle, "T");

					nk_layout_row_dynamic(ctx, 0, 2);
					nk_button_label_styled(ctx, &textBackgroundStyle, "Toggle Settings");
					nk_button_label_styled(ctx, &textBackgroundStyle, "S");

				} else if (page == 1) {
					nk_style_set_font(ctx, &font_large->handle);
					nk_layout_row_dynamic(ctx, 0, 1);
					nk_button_label_styled(ctx, &textBackgroundStyle, "Presets Page");

				} else if (page == 2) {
					nk_style_set_font(ctx, &font_large->handle);
					nk_layout_row_dynamic(ctx, 50, 1);
					if (nk_button_label_styled(ctx, &textBackgroundStyle, "Exit Program")) glfwSetWindowShouldClose(window, true);
				}
			} nk_end(ctx);
			nk_style_pop_float(ctx);
			nk_style_pop_color(ctx);
			nk_style_pop_style_item(ctx);
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
			if (updateWallList->obj.id == -1) {
				updateWallList->obj.position[1] = height;
			} else if (updateWallList->obj.id == -2) {
				updateWallList->obj.position[1] = terminal ? terminalHeight : 0.0f;
			} else if (updateWallList->obj.id == -4 != 0.0f) {
				updateWallList->obj.position[0] = navbar ? width - navbarWidth : width;
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
						calcCollision(&traverseObjectList->obj, &traverseWallList->obj, coefficientOfRestitutionWall);
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
						calcCollision(&traverseObjectList->obj, &traverseObjectList2->obj, coefficientOfRestitutionObject);
					} traverseObjectList2 = traverseObjectList2->next;
				} 
				// If the object is out of bounds, delete the object
				if (traverseObjectList->obj.position[2] < 0) {
					deleteNode(objectList, traverseObjectList->obj.id);
				}
				moveObject(&traverseObjectList->obj, deltaTime, accelerationDueToGravity);
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
	nk_glfw3_key_callback(window, key, scancode, action, mods);
	
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		appendLinkedList(objectList, initialiseObject(id, (char*)"BALL", (vec3){200.0f, 100.0f, 0}, 10.0f, 10.0f, glm_rad(130.0f), 10.0f));
		id++;

	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
		togglePause();

	if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
		toggleNavbar();
	
	if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
		toggleTerminal();

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		//glfwSetWindowShouldClose(window, true);
		toggleSettings();
}

// Allows the user to resize the winodw
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
	glViewport(0, 0, width, height);
}

// Toggle flags
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

// Set global variable values
void setValue(char* value, char* identifier, int width, int height, Object* obj) {
	double num = atof(value); // This function ignores any characters inputted automatically
	if (identifier == "acc") {
		if (num < -10000.0f) num = -10000.0f;
		else if (num > 10000.0f) num = 10000.0f;
		accelerationDueToGravity = num;
	} else if (identifier == "id") {
		int num2 = (int)num;
		if (num2 < id) currentTerminalObject = num2;
	} else if (identifier == "posx") {
		if (num < 0.0f) num = 0.0f;
		else if (num > width) num = width - obj->radius;
		obj->position[0] = num;
	} else if (identifier == "posy") {
		if (num < 0.0f) num = 0.0f;
		else if (num > height) num = height - obj->radius;
		obj->position[1] = num;
	} else if (identifier == "rad") {
		if (num < 5.0f) num = 5.0f;
		else if (num > 300.0f) num = 300.0f;
		obj->radius = num;
	} else if (identifier == "spe") { 
		if (num < 0.0f) num = 0.0f;
		else if (num > 1000.0f) num = 1000.0f;
		obj->speed = num;
	} else if (identifier == "bea") {
		obj->bearing = normaliseBearing(num);
	} else if (identifier == "mass") {
		if (num < 0.001f) num = 0.001f;
		else if (num > 10000.0f) num = 10000.0f;
		obj->mass = num; 
	}
	else {
		printf("ERROR::SET_VALUE");
	} 	
	// memset(value, 0, sizeof(value));
	// value = (char*)"";

}

// Update the terminal display
void updateTerminal(struct nk_context *ctx, float currentFrame) {
	// Check if it is time to update the terminal
	if (currentFrame - lastUpdate >= printRate) {
		lastUpdate = currentFrame;

		// If there are too many entries, delete the oldest entry
		if (entryCount >= MAX_ENTRIES) {
			for (int i = 0; i < MAX_ENTRIES - 1; i++) {
				memcpy(entries[i], entries[i+1], 64);
			} entryCount = MAX_ENTRIES - 1;
		}

		// Find object that is being tracked
		Node* traverseObjectList = objectList;
		while (traverseObjectList!= NULL) {
			if (traverseObjectList->obj.id == currentTerminalObject) {
				break;
			} traverseObjectList = traverseObjectList->next;
		} 

		// Add new entry
		if (currentTerminalObjectProperty == speed) {
			snprintf(entries[entryCount], 64, "Speed: %f, Time: %f", traverseObjectList->obj.speed, currentFrame);
		} else if (currentTerminalObjectProperty == bearing) {
			snprintf(entries[entryCount], 64, "Bearing: %f, Time: %f", traverseObjectList->obj.bearing, currentFrame);
		} else if (currentTerminalObjectProperty == position) {
			snprintf(entries[entryCount], 64, "Position: %f, %f, Time: %f", traverseObjectList->obj.position[0], traverseObjectList->obj.position[1], currentFrame);
		} else {
			return;
		} entryCount++;
	}
}