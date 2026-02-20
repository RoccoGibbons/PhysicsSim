#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main() {


    // Reads file
    FILE* vShaderFile = fopen("test.txt", "r");
    if (vShaderFile == NULL) {
        printf("ERROR::SHADER::VERTEX::FILE_NOT_READ\n");
    }

    // Sets a pointer to the end of the file
    // Finds the position of the end of the file and saves that value as the length of file
    // Sets the pointer back the the beginning of the file 
    fseek(vShaderFile, 0, SEEK_END);
    unsigned int vFileSize = ftell(vShaderFile);
    rewind(vShaderFile);

    // Sets a buffer for each line of the file and allocates memory for the entire file to be saved
    char buffer[128];
	char* vertexCode = (char*)malloc(vFileSize * sizeof(char));
    // Makes sure the allocated memory starts off as blank
    strcpy(vertexCode, "");

    // Concatenates buffer onto vertexCode for each line
    while(fgets(buffer, 128, vShaderFile) != NULL) {
		strcat(vertexCode, buffer);
	}

    // Frees the allocated memory
    printf("%s", vertexCode);
    free(vertexCode);
    vertexCode = NULL;

    return 0;
}