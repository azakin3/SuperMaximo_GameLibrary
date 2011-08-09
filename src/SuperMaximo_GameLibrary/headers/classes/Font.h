//============================================================================
// Name        : Font.h
// Author      : Max Foster
// Created on  : 30 May 2011
// Version     : 1.0
// Copyright   : http://creativecommons.org/licenses/by/3.0/
// Description : SuperMaximo GameLibrary Font class for drawing text
//============================================================================

#ifndef FONT_H_
#define FONT_H_

#include <iostream>
#include <SDL/SDL_ttf.h>
#include "../Display.h"

namespace SuperMaximo {

class Font {
	TTF_Font * font;
	unsigned size;
	std::string name_;
public:
	Font(std::string newName, std::string fileName, unsigned newSize);
	~Font();
	std::string name();
	void write(std::string text, int x, int y, float depth, bool useCache = true, float rotation = 0.0f, float xScale = 1.0f, float yScale = 1.0f);
	SDL_Surface * writeToSurface(std::string text, float r = 1.0f, float g = 1.0f, float b = 1.0f, bool hq = true);
	int width(std::string text);
	int height(std::string text);
	void cache(std::string text);
	void removeFromCache(std::string text);
};

class Shader;

void initFont(Shader * newFontShader);
void quitFont();
void bindFontShader(Shader * newFontShader);

Font * font(std::string searchName);
Font * addFont(std::string newName, std::string fileName, int newSize);
void destroyFont(std::string searchName);
void destroyAllFonts();

void clearFontCache();

}

#endif /* FONT_H_ */
