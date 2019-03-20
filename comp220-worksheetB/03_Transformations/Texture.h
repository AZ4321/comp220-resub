#pragma once

#include <SDL_image.h>
#include <GL\glew.h>
#include <SDL_opengl.h>

#include <string>

GLuint loadTextureFromFile(const std::string& filename);

