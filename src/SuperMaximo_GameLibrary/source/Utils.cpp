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

#include <SuperMaximo_GameLibrary/Utils.h>

namespace SuperMaximo {

int numCharInAlphabet(int letter) {
	if (letter > 96) letter -= 32;
	letter -= 65;
	if ((letter < 0) || (letter > 25)) letter = 26;
	return letter;
}

string leftStr(const string & str, unsigned amount) {
	string returnStr = "";
	if (amount > str.size()) amount = str.size();
	returnStr.reserve(amount);
	for (unsigned i = 0; i < amount; i++) returnStr += str[i];
	return returnStr;
}

string & leftStr(string * str, unsigned amount) {
	string tempStr = "";
	if (amount > str->size()) amount = str->size();
	tempStr.reserve(amount);
	for (unsigned i = 0; i < amount; i++) tempStr += (*str)[i];
	*str = tempStr;
	return *str;
}

string rightStr(const string & str, unsigned amount) {
	string returnStr = "";
	if (amount > str.size()) amount = str.size();
	returnStr.reserve(amount);
	for (unsigned i = 0; i < amount; i++) returnStr += str[(str.size()-amount)+i];
	return returnStr;
}

string & rightStr(string * str, unsigned amount) {
	string tempStr = "";
	if (amount > str->size()) amount = str->size();
	tempStr.reserve(amount);
	for (unsigned i = 0; i < amount; i++) tempStr += (*str)[((*str).size()-amount)+i];
	*str = tempStr;
	return *str;
}

string lowerCase(const string & str) {
	string returnStr = str;
	transform(returnStr.begin(), returnStr.end(), returnStr.begin(), ::tolower);
	return returnStr;
}

string & lowerCase(string * str) {
	transform(str->begin(), str->end(), str->begin(), ::tolower);
	return *str;
}

string upperCase(const string & str) {
	string returnStr = str;
	transform(returnStr.begin(), returnStr.end(), returnStr.begin(), ::toupper);
	return returnStr;
}

string & upperCase(string * str) {
	transform(str->begin(), str->end(), str->begin(), ::toupper);
	return *str;
}

}
