//============================================================================
// Name        : Sound.cpp
// Author      : Max Foster
// Created on  : 30 May 2011
// Version     : 1.0
// Copyright   : http://creativecommons.org/licenses/by/3.0/
// Description : SuperMaximo GameLibrary Sound class
//============================================================================

#include <iostream>
using namespace std;

#include <SDL/SDL_mixer.h>

#include <SuperMaximo_GameLibrary/classes/Sound.h>

namespace SuperMaximo {

vector<Sound*> Sound::channels;

Sound::Sound(const string & name, const string & fileName) :
		name_(name), chunk(Mix_LoadWAV(fileName.c_str())), volume_(100), currentChannel(-1) {}

Sound::~Sound() {
	Mix_FreeChunk(chunk);
}

const string & Sound::name() const {
	return name_;
}

int Sound::setVolume(int percentage, bool relative) {
	if (relative) volume_ += percentage; else volume_ = percentage;
	if (volume_ > 100) volume_ = 100; else if (volume_ < 0) volume_ = 0;
	if (currentChannel > -1) Mix_Volume(currentChannel, (MIX_MAX_VOLUME/100)*volume_);
	return volume_;
}

int Sound::volume() const{
	return volume_;
}

int Sound::play(int volume, int channel) {
	if (channel >= (int)channels.size()) return -1;

	currentChannel = Mix_PlayChannel(channel, chunk, 0);
	if (currentChannel > -1) {
		channels[currentChannel] = this;
		if (volume > -1) volume_ = volume;
		Mix_Volume(currentChannel, (MIX_MAX_VOLUME/100)*volume_);
	}
	return currentChannel;
}

void Sound::stop() const {
	if (Mix_GetChunk(currentChannel) == chunk) Mix_HaltChannel(currentChannel);
}

void Sound::setPosition(int angle, int distance) const {
	if (Mix_GetChunk(currentChannel) == chunk) Mix_SetPosition(currentChannel, angle, distance);
}

void Sound::allocateChannels(unsigned count) {
	Mix_AllocateChannels(count);
	channels.resize(count, NULL);
}

Sound * Sound::findByChannel(unsigned channel) {
	if (channel >= channels.size()) return NULL;
	return channels[channel];
}

}
