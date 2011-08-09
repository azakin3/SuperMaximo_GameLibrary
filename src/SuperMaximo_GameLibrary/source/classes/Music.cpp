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
#include "../../headers/classes/Music.h"
#include "../../headers/Utils.h"
using namespace SuperMaximo;

vector<Music*> allMusic[27];

namespace SuperMaximo {

Music::Music(string newName, string fileName) {
	name_ = newName, mixMusic = Mix_LoadMUS(fileName.c_str());
}

Music::~Music() {
	Mix_FreeMusic(mixMusic);
}

string Music::name() {
	return name_;
}

void Music::play() {
	Mix_PlayMusic(mixMusic, -1);
}

Music * music(string searchName) {
	int letter = numCharInAlphabet(searchName[0]);
	Music * returnMusic = NULL;
	if (allMusic[letter].size() > 0) {
		for (unsigned int i = 0; i < allMusic[letter].size(); i++) {
			if (allMusic[letter][i]->name() == searchName) returnMusic = allMusic[letter][i];
		}
	}
	return returnMusic;
}

Music * addMusic(string newName, string fileName) {
	int letter = numCharInAlphabet(newName[0]);
	Music * newMusic = new Music(newName, fileName);
	allMusic[letter].push_back(newMusic);
	return newMusic;
}

void destroyMusic(string searchName) {
	int letter = numCharInAlphabet(searchName[0]);
	if (allMusic[letter].size() > 0) {
		for (unsigned int i = 0; i < allMusic[letter].size(); i++) {
			if (allMusic[letter][i]->name() == searchName) {
				delete allMusic[letter][i];
				allMusic[letter].erase(allMusic[letter].begin()+i);
				break;
			}
		}
	}
}

void destroyAllMusic() {
	for (int i = 0; i < 27; i++) {
		if (allMusic[i].size() > 0) {
			for (unsigned int j = 0; j < allMusic[i].size(); j++) delete allMusic[i][j];
			allMusic[i].clear();
		}
	}
}

}
