#ifndef TEXTURE_H
#define TEXTURE_H

#include <glad/glad.h>
#include <string>

GLuint createTexture(const unsigned char* data, int width, int height, int nrChannels);
void bindTextureToShader(GLuint texture, GLuint textureUnit, GLuint shaderProgram, const std::string& uniformName);

#endif // TEXTURE_H