#include <glad/glad.h>
#include <cglm/cglm.h>
#include <stdio.h>
// #define NK_IMPLEMENTATION
#include <nuklear/nuklear.h>
#include <stb/stb_image.h>


void toggleNavbar() {
    
}

struct nk_image generateTexture(char* image) {
    int width, height, nrChannels;
    unsigned char *data = stbi_load(image, &width, &height, &nrChannels, 0);

    GLuint tex_id;
    glGenTextures(1, &tex_id);
    glBindTexture(GL_TEXTURE_2D, tex_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    return nk_image_id((int)tex_id);
}