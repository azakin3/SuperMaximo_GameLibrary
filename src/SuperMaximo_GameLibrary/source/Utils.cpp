//============================================================================
// Name        : Utils.cpp
// Author      : Max Foster
// Created on  : 6 May 2011
// Version     : 1.0
// Copyright   : http://creativecommons.org/licenses/by/3.0/
// Description : SuperMaximo GameLibrary useful functions
//============================================================================

#include <iostream>
#include <algorithm>
#include <string>
#include <sstream>
#include <cstdarg>
using namespace std;
#include "../headers/Utils.h"

namespace SuperMaximo {

int numCharInAlphabet(int letter) {
	if (letter > 96) letter -= 32;
	letter -= 65;
	if ((letter < 0) || (letter > 25)) letter = 26;
	return letter;
}

string leftStr(string str, int amount) {
	string returnStr = "";
	for (int i = 0; i < amount; i++) returnStr += str[i];
	return returnStr;
}

void leftStr(string * str, int amount) {
	string tempStr = "";
	for (int i = 0; i < amount; i++) tempStr += (*str)[i];
	*str = tempStr;
}

string rightStr(string str, int amount) {
	string returnStr = "";
	for (int i = 0; i < amount; i++) returnStr += str[(str.size()-amount)+i];
	return returnStr;
}

void rightStr(string * str, int amount) {
	string tempStr = "";
	for (int i = 0; i < amount; i++) tempStr += (*str)[((*str).size()-amount)+i];
	*str = tempStr;
}

string lowerCase(string str) {
	string returnStr = str;
	transform(returnStr.begin(), returnStr.end(), returnStr.begin(), ::tolower);
	return returnStr;
}

string upperCase(string str) {
	string returnStr = str;
	transform(returnStr.begin(), returnStr.end(), returnStr.begin(), ::toupper);
	return returnStr;
}

string intToStr(int i) {
	stringstream out;
	out << i;
	return out.str();
}

}
