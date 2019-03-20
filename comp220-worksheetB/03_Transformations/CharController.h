#pragma once
#include <iostream>

class CharController
{

public:
	CharController();
	CharController(InputManager* Input)

	void handleKeyboard(float deltaTime);

	void handleMouse();



private:

	InputManager*input;

	glm::vec3 cameraPosition;
	glm::vec3 cameraTarget;


	float movespeed;

};