//============================================================================
// Name        : Sound.h
// Author      : Max Foster
// Created on  : 30 May 2011
// Version     : 1.0
// Copyright   : http://creativecommons.org/licenses/by/3.0/
// Description : SuperMaximo GameLibrary Sound class
//============================================================================

#ifndef SOUND_H_
#define SOUND_H_

#include <iostream>
#include <SDL/SDL_mixer.h>

namespace SuperMaximo {

class Sound {
	Mix_Chunk * chunk;
	int volume_, currentChannel;
	std::string name_;
public:
	Sound(const std::string & newName, const std::string & fileName);
	~Sound();
	std::string name();
	void setVolume(int percentage, bool relative = false);
	int volume();
	int play(int newVolume = -1, int channel = -1);
	void stop();
	void setSoundPosition(int angle = 90, int distance = 0);
};

void allocateSoundChannels(unsigned channels);
Sound * findSoundByChannel(int channel);

}

#endif /* SOUND_H_ */
