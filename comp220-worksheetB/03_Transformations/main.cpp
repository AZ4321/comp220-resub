
#include <SDL.h>
#include <SDL_image.h>
#include <GL\glew.h>
#include <SDL_opengl.h>

#include <string>
#include <vector>
#include <fstream>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "vertex.h"
#include "shader.h"
#include "Texture.h"
#include "Model.h"

using namespace glm;

int main(int argc, char ** argsv)
{
	//Starting the SDL Library, using SDL_INIT_VIDEO to only run the video parts
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		//If SDL failed to initialize, this error will show
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "SDL_Init failed", SDL_GetError(), NULL);
		return 1;
	}
	if (IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG) < 0)
	{
		//If SDL failed to initialize, this error will show
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "SDL_Image failed", SDL_GetError(), NULL);
		return 1;
	}



	//Creating the window, have to remember to quit the window at the end to return the pointer by destroying it(the best way)
	SDL_Window* window = SDL_CreateWindow("SDL2 Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 640, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
	if (window == nullptr)
	{
		//Show error if SDL didnt create the window
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "SDL_CreateWindow failed", SDL_GetError(), NULL);

		SDL_Quit(); //quit SDL
		return 1;
	}

	//Requesting 3.3 Core OpenGL version
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	SDL_GLContext gl_Context = SDL_GL_CreateContext(window);
	if (gl_Context == nullptr) //if the glew context has a null value, show error and then destroy window
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "SDL_Create_Context failed, check to make sure the context is assigned correctly!", SDL_GetError(), NULL);
		SDL_DestroyWindow(window);
		SDL_Quit();

		return 1;

	}

	//Initialize GLEW
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (err != GLEW_OK) //if Glew is not okay, show error and close window
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Glew start-up failed, check to make sure Glew is started properly!", (char*)glewGetErrorString(err), NULL);
		SDL_DestroyWindow(window);
		SDL_Quit();

		return 1;
	}

	MeshCollection * tankMesh = new MeshCollection();
	loadMeshFromFile("Tank1.fbx", tankMesh);

	//Loading the image
	GLuint textureID = loadTextureFromFile("Tank1DF.png");

	//Triangle scale/position
	vec3 trianglePosition = vec3(0.0f, 0.0f, 0.0f);
	vec3 triangleScale = vec3(1.0f, 1.0f, 1.0f);
	vec3 triangleRotation = vec3(0.0f, 0.0f, 0.0f);

	//Correspodning matrices
	mat4 translationMatrix = translate(trianglePosition);
	mat4 scaleMatrix = scale(triangleScale);
	mat4 rotationMatrix = rotate(triangleRotation.x, vec3(1.0f, 0.0f, 0.0f))*rotate(triangleRotation.y, vec3(0.0f, 1.0f, 0.0f))*rotate(triangleRotation.z, vec3(0.0f, 0.0f, 1.0f));

	//Cumulating the above transformations into one 
	mat4 modelMatrix = translationMatrix * rotationMatrix*scaleMatrix;

	glm::mat4 matRoll = glm::mat4(1.0f);//identity matrix; 
	glm::mat4 matPitch = glm::mat4(1.0f);//identity matrix
	glm::mat4 matYaw = glm::mat4(1.0f);//identity matrix

	glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);

	//glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f); uncomment this and get rid of mouse camera stuff

	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

	//Directions


	//Mouse camera stuff
	void mouse_callback(SDL_Window* window, double xpos, double ypos)
	{
		if (firstMouse)
		{
			lastX = xpos;
			lastY = ypos;
			firstMouse = false;
		}

		float xoffset = xpos - lastX;
		float yoffset = lastY - ypos;
		lastX = xpos;
		lastY = ypos;

		float sensitivity = 0.05;
		xoffset *= sensitivity;
		yoffset *= sensitivity;

		yaw += xoffset;
		pitch += yoffset;


		if (pitch < -89.0f)
		{
			pitch = -89.0f;
		}

		if (pitch > 89.0f)
		{
			pitch = 89.0f;
		}

		glm::vec3 front;
		front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		front.y = sin(glm::radians(pitch));
		front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		cameraFront = glm::normalize(front);


	}



	float radius = 10.0f;
	//float camX = sin(glfwGetTime()) * radius;
	//float camZ = cos(glfwGetTime()) * radius; replace with SDL_GetTick

	glm::mat4 view;

	//Projection matrix
	mat4 projectionMatrix = perspective(radians(90.0f), float(800 / 600), 0.1f, 100.0f);

	//Light Colour
	glm::vec4 ambientLightColour = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	glm::vec4 diffuseLightColour = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

	//Light Direction
	glm::vec3 lightDirection = glm::vec3(0.0f, 0.0f, 1.0f);

	//Light Material Properties
	glm::vec4 ambientMaterialColour = glm::vec4(0.5f, 0.0f, 0.0f, 1.0f);
	glm::vec4 diffuseMaterialColour = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);



	GLint simpleProgramID = LoadShaders("blinnPhongVert.glsl", "blinnPhongFrag.glsl");
	if (simpleProgramID < 0)
	{
		printf("Textures have not loaded");
	}

	GLint modelMatrixLocation=glGetUniformLocation(simpleProgramID, "modelMatrix");
	GLint viewMatrixLocation = glGetUniformLocation(simpleProgramID, "viewMatrix");
	GLint projectionMatrixLocation = glGetUniformLocation(simpleProgramID, "projectionMatrix");
	GLint textureLocation = glGetUniformLocation(simpleProgramID, "baseTexture");

	//Light Uniform Location
	GLint ambientLightColourLocation = glGetUniformLocation(simpleProgramID, "ambientLightColour");
	GLint diffuseLightColourLocation = glGetUniformLocation(simpleProgramID, "diffuseLightColour");
	GLint lightDirectionLocation = glGetUniformLocation(simpleProgramID, "lightDirection");
	GLint ambientMaterialColourLocation = glGetUniformLocation(simpleProgramID, "ambientMaterialColour");
	GLint diffuseMaterialColourLocation = glGetUniformLocation(simpleProgramID, "diffuseMaterialColour");


	//Running is always true as long as Escape is not pressed 
	bool running = true;
	float cameraSpeed = 0.05f;


	//SDL Event structure initiation
	SDL_Event ev;
	while (running)
	{
		//Poll for the events
		while (SDL_PollEvent(&ev))
		{
			//Switch case for event type
			switch (ev.type)
			{
				//If the case is SDL_QUIT then running will be false
			case SDL_QUIT:
				running = false;
				break;
				//Checks the keydown inputs
			case SDL_KEYDOWN:
				//Checks which button has been pressed
				switch (ev.key.keysym.sym)

				{
					//In case of ESC being pressed, the program will close
				case SDLK_ESCAPE:
					running = false;
					break;
				case SDLK_f:
					SDL_SetWindowFullscreen(window,1);
				case SDLK_w:
					cameraPos += cameraSpeed * cameraFront;
				case SDLK_s:
					cameraPos -= cameraSpeed * cameraFront;
				case SDLK_a:
					cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
				case SDLK_d:
					cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;

				}
			}
		}

		view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

		glEnable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		//Rendering goes here, noice
		glClearColor(0.0, 0.0, 0.0,1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//bind textures
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureID);

		glUseProgram(simpleProgramID);

		//Declare uniforms below
		glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, value_ptr(modelMatrix));
		glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, value_ptr(view));
		glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, value_ptr(projectionMatrix));
		glUniform1i(textureLocation, 0);

		//Sending light uniforms across
		glUniform4fv(ambientMaterialColourLocation, 1, glm::value_ptr(ambientMaterialColour));
		glUniform4fv(diffuseMaterialColourLocation, 1, glm::value_ptr(diffuseMaterialColour));

		glUniform4fv(ambientLightColourLocation, 1, glm::value_ptr(ambientLightColour));
		glUniform4fv(diffuseLightColourLocation, 1, glm::value_ptr(diffuseLightColour));

		glUniform3fv(lightDirectionLocation, 1, glm::value_ptr(lightDirection));

		tankMesh->render();

		SDL_GL_SwapWindow(window);
		SDL_SetWindowResizable(window, SDL_TRUE);
	}
	if (tankMesh)
	{
		tankMesh->destroy();
		delete tankMesh;
		tankMesh = nullptr;
	}
	glDeleteTextures(1, &textureID);
	glDeleteProgram(simpleProgramID);
	//Deleting the context
	SDL_GL_DeleteContext(gl_Context);
	//Clean up, deactivating the library and the window
	SDL_DestroyWindow(window);
	IMG_Quit();
	SDL_Quit();

	return 0;

}

