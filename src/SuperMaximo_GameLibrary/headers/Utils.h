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
#include <sstream>
#include <math.h>

namespace SuperMaximo {

const double pi = 3.14159265358979323846;

int numCharInAlphabet(int letter);

std::string leftStr(const std::string & str, unsigned amount);

std::string & leftStr(std::string * str, unsigned amount);

std::string rightStr(const std::string & str, unsigned amount);

std::string & rightStr(std::string * str, unsigned amount);

std::string lowerCase(const std::string & str);

std::string & lowerCase(std::string * str);

std::string upperCase(const std::string & str);

std::string & upperCase(std::string * str);

template <typename type>
std::string toString(const type & variable) {
	std::stringstream stream;
	stream << variable;
	return stream.str();
}

template<typename type>
type degToRad(type angle) {
	return (angle*pi)/type(180.0);
}

template<typename type>
type radToDeg(type angle) {
	return (angle*type(180.0))/pi;
}

}

#endif /* UTILS_H_ */
