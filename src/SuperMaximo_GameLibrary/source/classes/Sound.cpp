//============================================================================
// Name        : Sound.cpp
// Author      : Max Foster
// Created on  : 30 May 2011
// Version     : 1.0
// Copyright   : http://creativecommons.org/licenses/by/3.0/
// Description : SuperMaximo GameLibrary Sound class
//============================================================================

#include <iostream>
#include <vector>
using namespace std;
#include <SDL/SDL_mixer.h>
#include "../../headers/classes/Sound.h"
#include "../../headers/Utils.h"
using namespace SuperMaximo;

vector<Sound*> channels;

namespace SuperMaximo {

void allocateSoundChannels(unsigned channels) {
	Mix_AllocateChannels(channels);
	unsigned i = ::channels.size();
	::channels.reserve(channels);
	while (i > channels) {
		::channels.pop_back();
		i--;
	}
	while (i < channels) {
		::channels.push_back(NULL);
		i++;
	}
}

Sound::Sound(const string & newName, const string & fileName) {
	name_ = newName, chunk = Mix_LoadWAV(fileName.c_str()), volume_ = 100, currentChannel = -1;
}

Sound::~Sound() {
	Mix_FreeChunk(chunk);
}

string Sound::name() {
	return name_;
}

void Sound::setVolume(int percentage, bool relative) {
	if (relative) volume_ += percentage; else volume_ = percentage;
	if (volume_ > 100) volume_ = 100; else if (volume_ < 0) volume_ = 0;
	if (currentChannel > -1) Mix_Volume(currentChannel, (MIX_MAX_VOLUME/100)*volume_);
}

int Sound::volume() {
	return volume_;
}

int Sound::play(int newVolume, int channel) {
	if (channel >= (int)channels.size()) return -1;
	currentChannel = Mix_PlayChannel(channel, chunk, 0);
	if (currentChannel > -1) {
		channels[currentChannel] = this;
		if (newVolume > -1) volume_ = newVolume;
		Mix_Volume(currentChannel, (MIX_MAX_VOLUME/100)*volume_);
	}
	return currentChannel;
}

void Sound::stop() {
	if (Mix_GetChunk(currentChannel) == chunk) Mix_HaltChannel(currentChannel);
}

void Sound::setSoundPosition(int angle, int distance) {
	if (Mix_GetChunk(currentChannel) == chunk) Mix_SetPosition(currentChannel, angle, distance);
}

Sound * findSoundByChannel(int channel) {
	return channels[channel];
}

}
