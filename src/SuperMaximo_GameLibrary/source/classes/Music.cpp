//============================================================================
// Name        : Music.cpp
// Author      : Max Foster
// Created on  : 30 May 2011
// Version     : 1.0
// Copyright   : http://creativecommons.org/licenses/by/3.0/
// Description : SuperMaximo GameLibrary Music class
//============================================================================

#include <iostream>
#include <vector>
using namespace std;

#include <SDL/SDL_mixer.h>

#include <SuperMaximo_GameLibrary/classes/Music.h>

namespace SuperMaximo {

int Music::volume_ = 100;

Music::Music(const string & name, const string & fileName) : name_(name), mixMusic(Mix_LoadMUS(fileName.c_str())) {}

Music::~Music() {
	Mix_FreeMusic(mixMusic);
}

const string & Music::name() const {
	return name_;
}

void Music::play() const {
	Mix_PlayMusic(mixMusic, -1);
	Mix_VolumeMusic((MIX_MAX_VOLUME/100)*volume_);
}

int Music::volume() {
	return volume_;
}

int Music::setVolume(int percentage, bool relative) {
	if (relative) volume_ += percentage; else volume_ = percentage;
	if (volume_ > 100) volume_ = 100; else if (volume_ < 0) volume_ = 0;
	Mix_VolumeMusic((MIX_MAX_VOLUME/100)*volume_);
	return volume_;
}

void Music::pause() {
	Mix_PauseMusic();
}

void Music::resume() {
	Mix_ResumeMusic();
}

void Music::restart() {
	Mix_RewindMusic();
}

void Music::stop() {
	Mix_HaltMusic();
}

void Music::fadeOut(unsigned time) {
	Mix_FadeOutMusic(time);
}

}
