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

vector<Sound*> allSounds[27];
vector<Sound*> allChannels;

namespace SuperMaximo {

void allocateSoundChannels(unsigned channels) {
	Mix_AllocateChannels(channels);
	unsigned i = allChannels.size();
	while (i > channels) {
		allChannels.pop_back();
		i--;
	}
	while (i < channels) {
		allChannels.push_back(NULL);
		i++;
	}
}

Sound::Sound(string newName, string fileName) {
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
	currentChannel = Mix_PlayChannel(channel, chunk, 0);
	if (currentChannel > -1) {
		allChannels[currentChannel] = this;
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

Sound * sound(string searchName) {
	int letter = numCharInAlphabet(searchName[0]);
	Sound * returnSound = NULL;
	if (allSounds[letter].size() > 0) {
		for (unsigned int i = 0; i < allSounds[letter].size(); i++) {
			if (allSounds[letter][i]->name() == searchName) returnSound = allSounds[letter][i];
		}
	}
	return returnSound;
}

Sound * sound(int channel) {
	return allChannels[channel];
}

Sound * addSound(string newName, string fileName) {
	int letter = numCharInAlphabet(newName[0]);
	Sound * newSound = new Sound(newName, fileName);
	allSounds[letter].push_back(newSound);
	return newSound;
}

void destroySound(std::string searchName) {
	int letter = numCharInAlphabet(searchName[0]);
	if (allSounds[letter].size() > 0) {
		for (unsigned int i = 0; i < allSounds[letter].size(); i++) {
			if (allSounds[letter][i]->name() == searchName) {
				delete allSounds[letter][i];
				allSounds[letter].erase(allSounds[letter].begin()+i);
				break;
			}
		}
	}
}

void destroySound(int channel) {
	if (allChannels[channel] != NULL) {
		int letter = numCharInAlphabet(allChannels[channel]->name()[0]);
		if (allSounds[letter].size() > 0) {
			for (unsigned int i = 0; i < allSounds[letter].size(); i++) {
				if (allSounds[letter][i]->name() == allChannels[channel]->name()) {
					delete allSounds[letter][i];
					allSounds[letter].erase(allSounds[letter].begin()+i);
					allChannels[channel] = NULL;
					break;
				}
			}
		}
	}
}

void destroyAllSounds() {
	for (int i = 0; i < 27; i++) {
		if (allSounds[i].size() > 0) {
			for (unsigned int j = 0; j < allSounds[i].size(); j++) delete allSounds[i][j];
			allSounds[i].clear();
		}
	}
	for (unsigned i = 0; i < allChannels.size(); i++) allChannels[i] = NULL;
}

}
