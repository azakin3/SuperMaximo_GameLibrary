//============================================================================
// Name        : SMSDL.cpp
// Author      : Max Foster
// Created on  : 5 May 2011
// Version     : 1.0
// Copyright   : http://creativecommons.org/licenses/by/3.0/
// Description : SuperMaximo GameLibrary SDL initialisation functions
//============================================================================

#include <SDL/SDL.h>
#include <SDL/SDL_stdinc.h>

#include <SuperMaximo_GameLibrary/SMSDL.h>

namespace SuperMaximo {

void initSDL(Uint32 flags) {
	if (flags != SDL_INIT_NOPARACHUTE) SDL_Init(flags | SDL_INIT_NOPARACHUTE); else SDL_Init(SDL_INIT_NOPARACHUTE);
}

void quitSDL() {
	SDL_Quit();
}

void wait(long time) {
	SDL_Delay(time);
}

}
