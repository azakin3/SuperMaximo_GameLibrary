//============================================================================
// Name        : GameHelpers.cpp
// Author      : Max Foster
// Created on  : 10 May 2011
// Version     : 1.0
// Copyright   : http://creativecommons.org/licenses/by/3.0/
// Description : SuperMaximo GameLibrary functions to make game creation easier
//============================================================================

#include <iostream>
#include <cmath>
using namespace std;

#include "../headers/GameHelpers.h"
#include "../headers/Display.h"
#include "../headers/Input.h"
#include "../headers/Utils.h"
using namespace SuperMaximo;

float rotX = 0, rotY = 0, rotZ = 0;
float yRot;
int lastMouseX = -1, lastMouseY = -1;
bool mouseRelocated = false;

namespace SuperMaximo {

void firstPersonMouseControlView(float sensitivity) {
	if (!mouseRelocated) {
		setMousePosition(screenWidth()/2, screenHeight()/2);
		yRot = 0;
		mouseRelocated = true;
	}
	sensitivity = 100.0f-(sensitivity*10.0f);
	if (sensitivity == 0.0f) sensitivity = 0.1f;

	resetEvents();
	int mouseXDiff = mouseX()-(screenWidth()/2);
	setMousePosition(screenWidth()/2, mouseY());
	lastMouseX -= mouseXDiff;

	resetEvents();
	int mouseYDiff = mouseY()-(screenHeight()/2);
	setMousePosition(mouseX(), screenHeight()/2);
	lastMouseY -= mouseYDiff;

	float yRotAddition = float(mouseX()-lastMouseX)/sensitivity;
	rotY += yRotAddition;
	lastMouseX = mouseX();
	rotX += cos(degToRad(yRot))*(float(mouseY()-lastMouseY)/sensitivity);
	rotZ += sin(degToRad(yRot))*(float(mouseY()-lastMouseY)/sensitivity);
	lastMouseY = mouseY();

	setMatrix(MODELVIEW_MATRIX);
	rotateMatrix(rotX, 1, 0, 0);
	rotateMatrix(rotY, 0, 1, 0);
	rotateMatrix(rotZ, 0, 0, 1);
	yRot += yRotAddition;
}

}

