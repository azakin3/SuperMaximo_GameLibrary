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
#include <vector>

struct Mix_Chunk;

namespace SuperMaximo {

class Sound {
	static std::vector<Sound*> channels;

	std::string name_;
	Mix_Chunk * chunk;
	int volume_, currentChannel;

public:
	Sound(const std::string & name, const std::string & fileName);
	~Sound();

	const std::string & name() const;
	int volume() const;
	int setVolume(int percentage, bool relative = false);
	int play(int volume = -1, int channel = -1);
	void stop() const;
	void setPosition(int angle = 90, int distance = 0) const;

	static void allocateChannels(unsigned count);
	static Sound * findByChannel(unsigned channel);
};

}

#endif /* SOUND_H_ */
