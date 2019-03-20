#include "Texture.h"

GLuint loadTextureFromFile(const std::string& filename)
{
	GLuint textureID;

	SDL_Surface* surface = IMG_Load(filename.c_str());
	if (surface == nullptr)
	{
		printf("Could not load image file");
		return 0;

	}

	//assigning surface
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	SDL_FreeSurface(surface);

	return textureID;
}
