
#include <SDL.h>
#include <SDL_image.h>
#include <GL\glew.h>
#include <SDL_opengl.h>
#include <iostream>

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
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
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

	//Capture mouse using SDL to make sure it stays on the screen
	SDL_CaptureMouse(SDL_TRUE);

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

	//Loading the texture
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

	//Defining cameraPos, cameraFront, CameraUp to use for key controls/mouse movement
	glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
	glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

	//Last mouse co-ords
	float lastX = 400, lastY = 320;

	//Mouse yaw/pitch
	float yaw = 0;
	float pitch = 0;

	//Radius
	float radius = 10.0f;

	//Declaring view as a mat4
	glm::mat4 view;

	//Projection matrix
	mat4 projectionMatrix = perspective(radians(90.0f), float(800 / 600), 0.1f, 100.0f);

	//Light Colour Properties
	glm::vec4 ambientLightColour = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	glm::vec4 diffuseLightColour = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	glm::vec4 specularLightColour = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

	//Light Direction
	glm::vec3 lightDirection = glm::vec3(0.0f, 0.0f, 1.0f);

	//Light Material Properties
	glm::vec4 ambientMaterialColour = glm::vec4(0.5f, 0.0f, 0.0f, 1.0f);
	glm::vec4 diffuseMaterialColour = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	glm::vec4 specularMaterialColour = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	float specularMaterialPower = 25.0f;

	//Loading shaders, if not print error
	GLint simpleProgramID = LoadShaders("blinnPhongVert.glsl", "blinnPhongFrag.glsl");
	if (simpleProgramID < 0)
	{
		printf("Shaders have not loaded");
	}

	//Uniform Locations for the model, view and projection matrixes as well as the location for the texture
	GLint modelMatrixLocation=glGetUniformLocation(simpleProgramID, "modelMatrix");
	GLint viewMatrixLocation = glGetUniformLocation(simpleProgramID, "viewMatrix");
	GLint projectionMatrixLocation = glGetUniformLocation(simpleProgramID, "projectionMatrix");
	GLint textureLocation = glGetUniformLocation(simpleProgramID, "baseTexture");

	//Getting the Light Colour location uniforms as well as the Light Direction
	GLint ambientLightColourLocation = glGetUniformLocation(simpleProgramID, "ambientLightColour");
	GLint diffuseLightColourLocation = glGetUniformLocation(simpleProgramID, "diffuseLightColour");
	GLint specularLightColourLocation = glGetUniformLocation(simpleProgramID, "specularLightColour");
	GLint lightDirectionLocation = glGetUniformLocation(simpleProgramID, "lightDirection");

	//Uniform for the light material colour locations to pass in later
	GLint ambientMaterialColourLocation = glGetUniformLocation(simpleProgramID, "ambientMaterialColour");
	GLint diffuseMaterialColourLocation = glGetUniformLocation(simpleProgramID, "diffuseMaterialColour");
	GLint specularMaterialColourLocation = glGetUniformLocation(simpleProgramID, "specularMaterialColour");
	GLint specularMaterialPowerLocation = glGetUniformLocation(simpleProgramID, "specularMaterialPower");

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
			case SDL_MOUSEMOTION:
			{
				//Calculate mouse x/y
				float x = ev.motion.x;
				float y = ev.motion.y;
				float xoffset = x - lastX;
				float yoffset = lastY - y; // reversed since y-coordinates range from bottom to top
				//stating the last x/y mouse co-ords to be the present ones
				lastX = x;
				lastY = y;
				//Sensitivity
				float sensitivity = 0.05f;
				xoffset *= sensitivity;
				yoffset *= sensitivity;

				//yaw/pitch offsets
				yaw += xoffset;
				pitch += yoffset;
				 
				//Clamps pitch
				if (pitch > 89.0f)
					pitch = 89.0f;
				if (pitch < -89.0f)
					pitch = -89.0f;

				//Calculating look direction from yaw/pitch
				glm::vec3 front;
				front.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
				front.y = sin(glm::radians(pitch));
				front.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
				cameraFront = glm::normalize(front);

				std::cout << "Mouse Position " << x <<" "<< y << std::endl;
				break;

			}
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
					SDL_SetWindowFullscreen(window,1); //setting window to fullscreen on F key press
				case SDLK_w:
					cameraPos += cameraSpeed * cameraFront; //If W key is pressed, the camera pos is more than or equal to the speed * front
					break;
				case SDLK_s:
					cameraPos += -cameraSpeed * cameraFront; //If S key is pressed, the camera pos is more than or equal to the speed * front
					break;
				case SDLK_a:
					cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
					break;
				case SDLK_d:
					cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
					break;

				}
			}
		}
		//Declaring the view to take in all the camera components
		view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

		glEnable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		//Rendering goes here, noice
		glClearColor(0.0, 0.0, 0.0,1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//bind textures
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureID);

		//Setting programID
		glUseProgram(simpleProgramID);

		//Passing in uniforms below
		glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, value_ptr(modelMatrix));
		glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, value_ptr(view));
		glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, value_ptr(projectionMatrix));
		glUniform1i(textureLocation, 0);

		//Sending light material colour locations across
		glUniform4fv(ambientMaterialColourLocation, 1, glm::value_ptr(ambientMaterialColour));
		glUniform4fv(diffuseMaterialColourLocation, 1, glm::value_ptr(diffuseMaterialColour));
		glUniform4fv(specularMaterialColourLocation, 1, glm::value_ptr(specularMaterialColour));

		//Sending light colour locations across
		glUniform4fv(ambientLightColourLocation, 1, glm::value_ptr(ambientLightColour));
		glUniform4fv(diffuseLightColourLocation, 1, glm::value_ptr(diffuseLightColour));
		glUniform4fv(specularLightColourLocation, 1, glm::value_ptr(specularLightColour));

		////Sending specular material power location accross, along with the value of the specular material power
		glUniform1f(specularMaterialPowerLocation, specularMaterialPower);

		//Sending light direction location accross with its value
		glUniform3fv(lightDirectionLocation, 1, glm::value_ptr(lightDirection));

		//Render mesh
		tankMesh->render();

		//Setting window to be resizable
		SDL_GL_SwapWindow(window);
		SDL_SetWindowResizable(window, SDL_TRUE);
	}
	if (tankMesh)
	{
		tankMesh->destroy();
		delete tankMesh;
		tankMesh = nullptr;
	}

	//Cleanup
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

