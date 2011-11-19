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
#include <SDL/SDL_mixer.h>

namespace SuperMaximo {

class Music {
	Mix_Music * mixMusic;
	std::string name_;
public:
	Music(const std::string & newName, const std::string & fileName);
	~Music();
	std::string name();
	void play();
};

Music * music(const std::string & searchName);
Music * addMusic(const std::string & newName, const std::string & fileName);
void destroyMusic(const std::string & searchName);
void destroyAllMusic();

}

#endif /* MUSIC_H_ */
