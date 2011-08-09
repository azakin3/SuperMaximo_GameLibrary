//============================================================================
// Name        : Utils.h
// Author      : Max Foster
// Created on  : 6 May 2011
// Version     : 1.0
// Copyright   : http://creativecommons.org/licenses/by/3.0/
// Description : SuperMaximo GameLibrary useful functions
//============================================================================

#ifndef UTILS_H_
#define UTILS_H_

#include <iostream>
#include <math.h>
#include "Display.h"

namespace SuperMaximo {

int numCharInAlphabet(int letter);

std::string leftStr(std::string str, int amount);

void leftStr(std::string * str, int amount);

std::string rightStr(std::string str, int amount);

void rightStr(std::string * str, int amount);

std::string lowerCase(std::string str);

std::string upperCase(std::string str);

std::string intToStr(int i);

template<typename number>
number degToRad(number angle) {
	return (angle*M_PI)/number(180.0);
}

template<typename number>
number radToDeg(number angle) {
	return (angle*number(180.0))/M_PI;
}

}

#endif /* UTILS_H_ */
