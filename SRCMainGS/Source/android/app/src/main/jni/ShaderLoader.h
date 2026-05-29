#pragma once

#include <GLES3/gl3.h>

namespace ShaderLoader {

// Compile and link the default shader program (vertex + fragment)
GLuint loadDefaultProgram();

// Load a shader from assets
GLuint loadProgram(const char* vertAssetPath, const char* fragAssetPath);

// Compile a single shader from source
GLuint compileShader(GLenum type, const char* source);

// Link a program from vertex + fragment shaders
GLuint linkProgram(GLuint vertShader, GLuint fragShader);

// Release all cached programs
void shutdown();

} // namespace ShaderLoader
