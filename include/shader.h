#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <string>

std::string readShaderFile(const std::string& filePath);
GLuint compileShader(GLenum shaderType, const std::string& shaderSource);
GLuint createShaderProgram(const std::string& vertexPath, const std::string& fragmentPath);
#endif // SHADER_H