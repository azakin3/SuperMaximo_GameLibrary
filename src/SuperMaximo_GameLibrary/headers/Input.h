//============================================================================
// Name        : Input.h
// Author      : Max Foster
// Created on  : 5 May 2011
// Version     : 1.0
// Copyright   : http://creativecommons.org/licenses/by/3.0/
// Description : SuperMaximo GameLibrary input functions
//============================================================================

#ifndef INPUT_H_
#define INPUT_H_

namespace SuperMaximo {

enum dpadEnum {
	DPAD_UP = 0,
	DPAD_DOWN,
	DPAD_LEFT,
	DPAD_RIGHT
};

//Initialises Input
void initInput();
//Quits Input
void quitInput();

//Returns true if the key specified by the keycode is currently being pressed
bool keyPressed(unsigned code);
//Sets the position of the cursor to the coordinates specified
void setMousePosition(int x, int y);
//Hides the cursor
void hideCursor();
//Shows the cursor
void showCursor();
//Returns the X value of the cursor relative to the Display window
int mouseX();
//Returns the Y value of the cursor relative to the Display window
int mouseY();
//If the left mouse button is down then true is returned
bool mouseLeft();
//If the right mouse button is down then true is returned
bool mouseRight();
//If the middle mouse button is down then true is returned
bool mouseMiddle();
//If a mouse button other than the left, right or middle button is down then true is returned
bool mouseOther();
//If the mouse wheel is being scrolled up, then true is returned
bool mouseWheelUp();
//If the mouse wheel is being scrolled down, then true is returned
bool mouseWheelDown();

bool closeClicked();

int joystickCount();

bool joystickButtonPressed(unsigned code, unsigned controllerId = 0);

bool joystickDpadPressed(dpadEnum code, unsigned controllerId = 0);

int joystickAxisValue(unsigned axis, unsigned controllerId = 0);

void resetEvents();

}

#endif /* INPUT_H_ */
