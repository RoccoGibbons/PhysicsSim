#include <glad/glad.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    unsigned int ID;
} Shader;

Shader initialise_shader(const char* vertexPath, const char* fragmentPath);
int length(char* myString);
void checkCompileErrors(unsigned int shader, char* type);
void use(Shader myShader);
// Need to create uniform setter functions.

Shader initialise_shader(const char* vertexPath, const char* fragmentPath) {
    Shader myShader;
    
    FILE* vShaderFile = fopen(vertexPath, "r");
    if (vShaderFile == NULL) {
        printf("ERROR::SHADER::VERTEX::FILE_NOT_READ");
    }

    FILE* fShaderFile = fopen(fragmentPath, "r");
    if (fShaderFile == NULL) {
        printf("ERROR::SHADER::FRAGMENT::FILE_NOT_READ");
    }

    char buffer[256];
    char* vertexCode;
    while(fgets(buffer, 256, vShaderFile)) {
        strncat(vertexCode, buffer, length(buffer));
    }

    char buffer2[256];
    char* fragmentCode;
    while(fgets(buffer, 256, fShaderFile)) {
        strncat(fragmentCode, buffer2, length(buffer));
    }

    unsigned int vertex, fragment;
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vertexCode, NULL);
    glCompileShader(vertex);
    checkCompileErrors(vertex, "VERTEX");

    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fragmentCode, NULL);
    glCompileShader(fragment);
    checkCompileErrors(fragment, "FRAGMENT");

    myShader.ID = glCreateProgram();
    glAttachShader(myShader.ID, vertex);
    glAttachShader(myShader.ID, fragment);
    glLinkProgram(myShader.ID);
    checkCompileErrors(myShader.ID, "PROGRAM");

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    return myShader;
}

int length(char* myString) {
    int counter = 0;
    while (myString[counter] != '\n') {
        counter += 1;
    }
    return counter;
}

void checkCompileErrors(unsigned int shader, char* type) {
    int success;
    char infoLog[512];
    if (type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 512, NULL, infoLog);
            printf("ERROR::SHADER::%s::COMPILATION_FAILED %s /n", type, infoLog);
        }
    }
    else {
        glGetProgramiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 512, NULL, infoLog);
            printf("ERROR::SHADER::%s::LINK_FAILED %s /n", type, infoLog);
        }
    }
}

void use(Shader myShader) {
    glUseProgram(myShader.ID);
}