//============================================================================
// Name        : Audio.cpp
// Author      : Max Foster
// Created on  : 8 May 2011
// Version     : 1.0
// Copyright   : http://creativecommons.org/licenses/by/3.0/
// Description : SuperMaximo GameLibrary audio functions
//============================================================================

#include <SDL/SDL_mixer.h>
#include "../headers/Audio.h"
#include "../headers/classes/Sound.h"

namespace SuperMaximo {

void initAudio(unsigned channels) {
	 Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
	 allocateSoundChannels(channels);
}

void quitAudio() {
	Mix_CloseAudio();
}

void soundPosition(int channel, int angle, int distance) {
	Mix_SetPosition(channel, angle, distance);
}

void musicVolume(int percentage) {
	Mix_VolumeMusic((MIX_MAX_VOLUME/100)*percentage);
}

void pauseMusic() {
	Mix_PauseMusic();
}

void resumeMusic() {
	Mix_ResumeMusic();
}

void restartMusic() {
	Mix_RewindMusic();
}

void stopMusic() {
	Mix_HaltMusic();
}

void fadeMusic(long int time) {
	Mix_FadeOutMusic(time);
}

}
