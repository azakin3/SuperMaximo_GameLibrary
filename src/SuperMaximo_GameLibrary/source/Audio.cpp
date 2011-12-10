//============================================================================
// Name        : Audio.cpp
// Author      : Max Foster
// Created on  : 8 May 2011
// Version     : 1.0
// Copyright   : http://creativecommons.org/licenses/by/3.0/
// Description : SuperMaximo GameLibrary audio functions
//============================================================================

#include <SDL/SDL_mixer.h>

#include <SuperMaximo_GameLibrary/Audio.h>
#include <SuperMaximo_GameLibrary/classes/Sound.h>

namespace SuperMaximo {

void initAudio(unsigned channelCount) {
	 Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
	 Sound::allocateChannels(channelCount);
}

void quitAudio() {
	Mix_CloseAudio();
}

}
