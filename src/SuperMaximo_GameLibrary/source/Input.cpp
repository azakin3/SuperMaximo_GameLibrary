//============================================================================
// Name        : Input.cpp
// Author      : Max Foster
// Created on  : 5 May 2011
// Version     : 1.0
// Copyright   : http://creativecommons.org/licenses/by/3.0/
// Description : SuperMaximo GameLibrary input functions
//============================================================================

#include <vector>
using namespace std;

#include <SDL/SDL.h>
#include <SDL/SDL_events.h>

#include <SuperMaximo_GameLibrary/Input.h>
using namespace SuperMaximo;

struct joystick {
	SDL_Joystick * joystickHandle;
	unsigned id;
	bool buttons[16], dpad[4];
	Sint16 axis[16];
	void init(unsigned num) {
		id = num;
		joystickHandle = SDL_JoystickOpen(id);
		for (unsigned i = 0; i < 16; i++) {
			buttons[i] = false;
			axis[i] = 0;
		}
		for (unsigned i = 0; i < 4; i++) dpad[i] = false;
	}
};

static bool keys[320], mouseLeft_ = false, mouseRight_ = false, mouseMiddle_ = false, mouseOther_ = false,
		mouseWheelUp_ = false, mouseWheelDown_ = false, closeClicked_ = false, eventsRefreshed = false;
static int mouseX_, mouseY_, joystickCount_;
static vector <joystick> joysticks;

namespace SuperMaximo {

void initInput() {
	for (unsigned i = 0; i <= sizeof(keys); i++) keys[i] = false;
	SDL_JoystickEventState(SDL_ENABLE);
	joystickCount_ = SDL_NumJoysticks();
	for (int i = 0; i < joystickCount_; i++) {
		joystick newJoystick;
		newJoystick.init(i);
		joysticks.push_back(newJoystick);
	}
}

void quitInput() {
	for (unsigned i = 0; i < joysticks.size(); i++) SDL_JoystickClose(joysticks[i].joystickHandle);
	SDL_JoystickEventState(SDL_DISABLE);
}

void refreshEvents() {
	static SDL_Event event;

	mouseWheelUp_ = false;
	mouseWheelDown_ = false;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_KEYDOWN: keys[event.key.keysym.sym] = true; break;
		case SDL_KEYUP: keys[event.key.keysym.sym] = false; break;
		case SDL_MOUSEMOTION:
			mouseX_ = event.motion.x;
			mouseY_ = event.motion.y; break;
		case SDL_MOUSEBUTTONDOWN:
			switch (event.button.button) {
			case SDL_BUTTON_LEFT: mouseLeft_ = true; break;
			case SDL_BUTTON_RIGHT: mouseRight_ = true; break;
			case SDL_BUTTON_MIDDLE: mouseMiddle_ = true; break;
			case SDL_BUTTON_WHEELUP: mouseWheelUp_ = true; break;
			case SDL_BUTTON_WHEELDOWN: mouseWheelDown_ = true; break;
			default: mouseOther_ = true;
			}; break;
		case SDL_MOUSEBUTTONUP:
			switch (event.button.button) {
			case SDL_BUTTON_LEFT: mouseLeft_ = false; break;
			case SDL_BUTTON_RIGHT: mouseRight_ = false; break;
			case SDL_BUTTON_MIDDLE: mouseMiddle_ = false; break;
			default: mouseOther_ = false;
			}; break;
		case SDL_QUIT: closeClicked_ = true; break;
		case SDL_JOYBUTTONDOWN: joysticks[event.jbutton.which].buttons[event.jbutton.button] = true; break;
		case SDL_JOYBUTTONUP: joysticks[event.jbutton.which].buttons[event.jbutton.button] = false; break;
		case SDL_JOYHATMOTION:
			switch (event.jhat.value) {
			case SDL_HAT_UP:
				joysticks[event.jhat.which].dpad[0] = true;
				joysticks[event.jhat.which].dpad[1] = false;
				break;
			case SDL_HAT_DOWN:
				joysticks[event.jhat.which].dpad[1] = true;
				joysticks[event.jhat.which].dpad[0] = false;
				break;
			case SDL_HAT_LEFT:
				joysticks[event.jhat.which].dpad[2] = true;
				joysticks[event.jhat.which].dpad[3] = false;
				break;
			case SDL_HAT_RIGHT:
				joysticks[event.jhat.which].dpad[3] = true;
				joysticks[event.jhat.which].dpad[2] = false;
				break;
			case SDL_HAT_CENTERED:
				joysticks[event.jhat.which].dpad[0] = false;
				joysticks[event.jhat.which].dpad[1] = false;
				joysticks[event.jhat.which].dpad[2] = false;
				joysticks[event.jhat.which].dpad[3] = false;
				break;
			}
			break;
		case SDL_JOYAXISMOTION: joysticks[event.jaxis.which].axis[event.jaxis.axis] = event.jaxis.value; break;
		}
	}
	eventsRefreshed = true;
}

bool keyPressed(unsigned code) {
	if (!eventsRefreshed) refreshEvents();
	return keys[code];
}

void setMousePosition(int x, int y) {
	SDL_WarpMouse(x, y);
	mouseX_ = x;
	mouseY_ = y;
}

void hideCursor() {
	SDL_ShowCursor(0);
}

void showCursor() {
	SDL_ShowCursor(1);
}

int mouseX() {
	if (!eventsRefreshed) refreshEvents();
	return mouseX_;
}

int mouseY() {
	if (!eventsRefreshed) refreshEvents();
	return mouseY_;
}

bool mouseLeft() {
	if (!eventsRefreshed) refreshEvents();
	return mouseLeft_;
}

bool mouseRight() {
	if (!eventsRefreshed) refreshEvents();
	return mouseRight_;
}

bool mouseMiddle() {
	if (!eventsRefreshed) refreshEvents();
	return mouseMiddle_;
}

bool mouseOther() {
	if (!eventsRefreshed) refreshEvents();
	return mouseOther_;
}

bool mouseWheelUp() {
	if (!eventsRefreshed) refreshEvents();
	return mouseWheelUp_;
}

bool mouseWheelDown() {
	if (!eventsRefreshed) refreshEvents();
	return mouseWheelDown_;
}

bool closeClicked() {
	if (!eventsRefreshed) refreshEvents();
	return closeClicked_;
}

int joystickCount() {
	return joystickCount_;
}

bool joystickButtonPressed(unsigned code, unsigned controllerId) {
	if (joystickCount_ > 0) {
		if (!eventsRefreshed) refreshEvents();
		return joysticks[controllerId].buttons[code];
	} else return false;
}

bool joystickDpadPressed(dpadEnum code, unsigned controllerId) {
	if (joystickCount_ > 0) {
		if (!eventsRefreshed) refreshEvents();
		return joysticks[controllerId].dpad[code];
	} else return false;
}

int joystickAxisValue(unsigned axis, unsigned controllerId) {
	if (joystickCount_ > 0) {
		if (!eventsRefreshed) refreshEvents();
		return joysticks[controllerId].axis[axis];
	} else return 0;
}

void resetEvents() {
	eventsRefreshed = false;
}

}
/*
XBOX 360 Controller Maps

Hat - dpad

buttons:
0	A
1	B
2	X
3	Y
4	L bumper
5	R bumper
6	Start
7	XBox
8	L Analogue Click
9	R Analogue Click
10  Back

Axis
0	L Analogue - LEFT : RIGHT
1	L Analogue - TOP : BOTTOM
2	L Trigger - positive, R Trigger - minus
3	R Analogue - LEFT : RIGHT
4	R Analogue - TOP : BOTTOM
*/
