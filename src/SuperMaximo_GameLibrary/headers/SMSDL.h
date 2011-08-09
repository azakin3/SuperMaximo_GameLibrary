//============================================================================
// Name        : SMSDL.h
// Author      : Max Foster
// Created on  : 5 May 2011
// Version     : 1.0
// Copyright   : http://creativecommons.org/licenses/by/3.0/
// Description : SuperMaximo GameLibrary SDL initialisation functions
//============================================================================

#ifndef SDL_H_
#define SDL_H_

#include <SDL/SDL_stdinc.h>

namespace SuperMaximo {

#define SDL_INIT_TIMER	0x00000001
#define SDL_INIT_AUDIO	0x00000010
#define SDL_INIT_VIDEO	0x00000020
#define SDL_INIT_CDROM	0x00000100
#define SDL_INIT_JOYSTICK 0x00000200
#define SDL_INIT_NOPARACHUTE 0x00100000
#define SDL_INIT_EVENTTHREAD 0x01000000
#define SDL_INIT_EVERYTHING 0x0000FFFF

//Initialises everything from SDL with the specified flags shown in the constants section
void initSDL(Uint32 flags = SDL_INIT_NOPARACHUTE);
//Quits SDL
void quitSDL();
//Delays for a set amount of time
void wait(long time);

}

#endif /* SDL_H_ */
