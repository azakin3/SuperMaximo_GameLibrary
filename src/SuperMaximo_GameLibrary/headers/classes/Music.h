//============================================================================
// Name        : Music.h
// Author      : Max Foster
// Created on  : 30 May 2011
// Version     : 1.0
// Copyright   : http://creativecommons.org/licenses/by/3.0/
// Description : SuperMaximo GameLibrary Music class
//============================================================================

#ifndef MUSIC_H_
#define MUSIC_H_

#include <iostream>

struct _Mix_Music;

namespace SuperMaximo {

class Music {
	static int volume_;

	std::string name_;
	_Mix_Music * mixMusic;

public:
	Music(const std::string & newName, const std::string & fileName);
	~Music();

	const std::string & name() const;
	void play() const;

	static int volume();
	static int setVolume(int percentage, bool relative = false);

	static void pause();
	static void resume();
	static void restart();
	static void stop();
	static void fadeOut(unsigned time);
};

}

#endif /* MUSIC_H_ */
