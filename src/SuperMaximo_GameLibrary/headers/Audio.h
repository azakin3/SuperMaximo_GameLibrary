//============================================================================
// Name        : Audio.h
// Author      : Max Foster
// Created on  : 8 May 2011
// Version     : 1.0
// Copyright   : http://creativecommons.org/licenses/by/3.0/
// Description : SuperMaximo GameLibrary audio functions
//============================================================================

#ifndef AUDIO_H_
#define AUDIO_H_

namespace SuperMaximo {

void initAudio(unsigned channels = 16);
void quitAudio();

void soundPosition(int channel, int angle = 90, int distance = 0);

void musicVolume(int percentage);

void pauseMusic();
void resumeMusic();
void restartMusic();
void stopMusic();
void fadeMusic(int time);

}

#endif /* AUDIO_H_ */
