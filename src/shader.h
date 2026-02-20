#include <glad/glad.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct Shader {
    unsigned int ID;
} Shader;


Shader initialiseShader(const char* vertexPath, const char* fragmentPath);
unsigned int buildShaderPrograms(const char* path, const char* shaderType);
void checkCompileErrors(unsigned int shader, const char* type);
void use(Shader myShader);


Shader initialiseShader(const char* vertexPath, const char* fragmentPath) {
    Shader myShader;

    // Builds each individual shader program
    unsigned int vertexShader = buildShaderPrograms(vertexPath, "VERTEX");
    unsigned int fragmentShader = buildShaderPrograms(fragmentPath, "FRAGMENT");


	// Links the shader programs together
	myShader.ID = glCreateProgram();
	glAttachShader(myShader.ID, vertexShader);
	glAttachShader(myShader.ID, fragmentShader);
	glLinkProgram(myShader.ID);
    checkCompileErrors(myShader.ID, "PROGRAM");

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

    return myShader;
}

unsigned int buildShaderPrograms(const char* path, const char* shaderType) {
    // Reads file
    FILE* shaderFile = fopen(path, "r");
    if (shaderFile == NULL) {
        printf("ERROR::SHADER::%s::FILE_NOT_READ\n", shaderType);
    }
    
    // Sets a pointer to the end of the file
    // Finds the position of the end of the file and saves that value as the length of file
    // Sets the pointer back the the beginning of the file 
    fseek(shaderFile, 0, SEEK_END);
    unsigned int fileSize = ftell(shaderFile);
    rewind(shaderFile);
    
    // Sets a buffer for each line of the file and allocates memory for the entire file to be saved
    char buffer[128];
	char* shaderCode = (char*)malloc(fileSize * sizeof(char));
    // Makes sure the allocated memory starts off as blank
    strcpy(shaderCode, "");

    // Concatenates buffer onto shaderCode for each line
    while(fgets(buffer, 128, shaderFile) != NULL) {
		strcat(shaderCode, buffer);
	}

    fclose(shaderFile);
    
    // Build shader programs
	unsigned int shader;
    if (shaderType == "VERTEX") {
        shader = glCreateShader(GL_VERTEX_SHADER);
    }
    else if (shaderType == "FRAGMENT") {
        shader = glCreateShader(GL_FRAGMENT_SHADER);
    }
    else {
        printf("ERROR::SHADER::BUILD::UNKNOWN_SHADER_TYPE\n");
    }

    // Link the shader code to the actual shader and compile the shader
    glShaderSource(shader, 1, &shaderCode, NULL);
	glCompileShader(shader);
    checkCompileErrors(shader, shaderType);

    free(shaderCode);
    shaderCode = NULL;

    return shader;
}

void use(Shader myShader) {
    glUseProgram(myShader.ID);
}

void setBool(Shader myShader, const char* name, bool value) {
    glUniform1i(glGetUniformLocation(myShader.ID, name), (int)value);
}

void setInt(Shader myShader, const char* name, bool value) {
    glUniform1i(glGetUniformLocation(myShader.ID, name), value);
}

void setFloat(Shader myShader, const char* name, bool value) {
    glUniform1f(glGetUniformLocation(myShader.ID, name), value);
}

void checkCompileErrors(unsigned int shader, const char* shaderType) {
    int success;
    char infoLog[512];
    if (shaderType != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 512, NULL, infoLog);
            printf("ERROR::SHADER::%s::COMPILATION_FAILED %s /n", shaderType, infoLog);
        }
    }
    else {
        glGetProgramiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 512, NULL, infoLog);
            printf("ERROR::SHADER::%s::LINK_FAILED %s /n", shaderType, infoLog);
        }
    }
}
