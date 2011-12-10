//============================================================================
// Name        : Font.cpp
// Author      : Max Foster
// Created on  : 30 May 2011
// Version     : 1.0
// Copyright   : http://creativecommons.org/licenses/by/3.0/
// Description : SuperMaximo GameLibrary Font class for drawing text
//============================================================================

#include <iostream>
#include <vector>
using namespace std;
#include <GL/glew.h>
#include <SDL/SDL_ttf.h>
#include "../../headers/classes/Font.h"
#include "../../headers/classes/Shader.h"
#include "../../headers/Display.h"
#include "../../headers/Utils.h"
using namespace SuperMaximo;

struct fontCacheRecord {
	string text, fontName;
	GLuint texture;
	int w, h;
	GLuint vbo;
};

vector<fontCacheRecord> fontCache[27];
Shader * fontShader = NULL;
GLuint vbo = 0;

namespace SuperMaximo {

Font::Font(const string & newName, const string & fileName, unsigned newSize) {
	name_ = newName;
	size = newSize;
	font = TTF_OpenFont(fileName.c_str(), size);
}

Font::~Font() {
	if (font != NULL) TTF_CloseFont(font);
}

string Font::name() {
	return name_;
}

void Font::write(const string & text, int x, int y, float depth, bool useCache, float rotation, float xScale,
		float yScale) {
	bool cacheSuccess = false;
	unsigned cacheIndex = 1;
	int letter = numCharInAlphabet(text[0]);
	if (useCache) {
		if (fontCache[letter].size() > 0) {
			for (unsigned i = 0; i < fontCache[letter].size(); i++) {
				if ((fontCache[letter][i].text == text) && (fontCache[letter][i].fontName == name_)) {
					cacheIndex = i;
					cacheSuccess = true;
					break;
				}
			}
		}
		if (!cacheSuccess) {
			cache(text);
			for (unsigned i = 0; i < fontCache[letter].size(); i++) {
				if ((fontCache[letter][i].text == text) && (fontCache[letter][i].fontName == name_)) {
					cacheIndex = i;
					cacheSuccess = true;
					break;
				}
			}
		}
	}

	int w = 0, h = 0;
	SDL_Surface * textSurface = NULL;
	if (!cacheSuccess) {
		SDL_Color color;
		color.r = 255;
		color.g = 255;
		color.b = 255;
		textSurface = TTF_RenderText_Blended(font, text.c_str(), color);
		w = textSurface->w;
		h = textSurface->h;
	}
	GLenum textureFormat, textureType;
	if (textureRectangleDisabled()) textureType = GL_TEXTURE_2D; else textureType = GL_TEXTURE_RECTANGLE;
	GLuint tempTexture;
	glActiveTexture(GL_TEXTURE0);

	if (cacheSuccess) {
		glBindTexture(textureType, fontCache[letter][cacheIndex].texture);
		w = fontCache[letter][cacheIndex].w;
		h = fontCache[letter][cacheIndex].h;
		glBindBuffer(GL_ARRAY_BUFFER, fontCache[letter][cacheIndex].vbo);
		glVertexAttribPointer(VERTEX_ATTRIBUTE, 4, GL_FLOAT, GL_FALSE, 0, 0);
	} else {
		if (textSurface->format->BytesPerPixel == 4) {
			if (textSurface->format->Rmask == 0x000000ff) textureFormat = GL_RGBA; else textureFormat = GL_BGRA;
		} else {
			if (textSurface->format->Rmask == 0x000000ff) textureFormat = GL_RGB; else textureFormat = GL_BGR;
		}
		glGenTextures(1, &tempTexture);
		glBindTexture(textureType, tempTexture);
		glTexParameteri(textureType, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(textureType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(textureType, 0, textSurface->format->BytesPerPixel, w, h, 0, textureFormat, GL_UNSIGNED_BYTE,
				textSurface->pixels);

		GLfloat vertexArray[] = {
			0.0f, h, 0.0f, 1.0f,
			0.0f, 0.0f, 0.0f, 1.0f,
			w, 0.0f, 0.0f, 1.0f,

			0.0f, h, 0.0f, 1.0f,
			w, h, 0.0f, 1.0f,
			w, 0.0f, 0.0f, 1.0f};
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertexArray), vertexArray, GL_STATIC_DRAW);
		glVertexAttribPointer(VERTEX_ATTRIBUTE, 4, GL_FLOAT, GL_FALSE, 0, 0);
	}
	glEnableVertexAttribArray(VERTEX_ATTRIBUTE);

	glUseProgram(fontShader->program_);
	pushMatrix();
		translateMatrix(x, y, depth);
		rotateMatrix(rotation, 0.0f, 0.0f, 1.0f);
		scaleMatrix(xScale, yScale, 0.0f);

		fontShader->setUniform16(MODELVIEW_LOCATION, getMatrix(MODELVIEW_MATRIX));
		fontShader->setUniform16(PROJECTION_LOCATION, getMatrix(PROJECTION_MATRIX));
		fontShader->setUniform1(TEXSAMPLER_LOCATION, 0);

		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDisableVertexAttribArray(VERTEX_ATTRIBUTE);
	popMatrix();

	if (!cacheSuccess) {
		SDL_FreeSurface(textSurface);
		glDeleteTextures(1, &tempTexture);
		glDeleteBuffers(1, &vbo);
	}
}

SDL_Surface * Font::writeToSurface(const string & text, float r, float g, float b, bool hq) {
	SDL_Color color;
	color.r = r*255;
	color.g = g*255;
	color.b = b*255;
	SDL_Surface * returnedSurface;
	if (hq) returnedSurface = TTF_RenderText_Blended(font, text.c_str(), color);
		else returnedSurface = TTF_RenderText_Solid(font, text.c_str(), color);
	return returnedSurface;
}

int Font::width(const string & text) {
	int newWidth, newHeight;
	TTF_SizeText(font, text.c_str(), &newWidth, &newHeight);
	return newWidth;
}

int Font::height(const string & text) {
	int newWidth, newHeight;
	TTF_SizeText(font, text.c_str(), &newWidth, &newHeight);
	return newHeight;
}

void Font::cache(const string & text) {
	int letter = numCharInAlphabet(text[0]);
	fontCacheRecord newRecord;
	newRecord.text = text;
	newRecord.fontName = name_;
	TTF_SizeText(font, text.c_str(), &newRecord.w, &newRecord.h);

	SDL_Color color;
	color.r = 255;
	color.g = 255;
	color.b = 255;
	SDL_Surface * textSurface = TTF_RenderText_Blended(font, text.c_str(), color);
	GLenum textureFormat, textureType;
	if (textSurface->format->BytesPerPixel == 4) {
		if (textSurface->format->Rmask == 0x000000ff) textureFormat = GL_RGBA; else textureFormat = GL_BGRA;
	} else {
		if (textSurface->format->Rmask == 0x000000ff) textureFormat = GL_RGB; else textureFormat = GL_BGR;
	}
	if (textureRectangleDisabled()) textureType = GL_TEXTURE_2D; else textureType = GL_TEXTURE_RECTANGLE;

	glGenTextures(1, &newRecord.texture);
	glBindTexture(textureType, newRecord.texture);
	glTexParameteri(textureType, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(textureType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(textureType, 0, textSurface->format->BytesPerPixel, textSurface->w, textSurface->h, 0, textureFormat,
			GL_UNSIGNED_BYTE, textSurface->pixels);

	GLfloat vertexArray[] = {
		0.0f, textSurface->h, 0.0f, 1.0f,
		0.0f, 0.0f, 0.0f, 1.0f,
		textSurface->w, 0.0f, 0.0f, 1.0f,

		0.0f, textSurface->h, 0.0f, 1.0f,
		textSurface->w, textSurface->h, 0.0f, 1.0f,
		textSurface->w, 0.0f, 0.0f, 1.0f};
	SDL_FreeSurface(textSurface);

	glGenBuffers(1, &newRecord.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, newRecord.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexArray), vertexArray, GL_STATIC_DRAW);
	glVertexAttribPointer(VERTEX_ATTRIBUTE, 4, GL_FLOAT, GL_FALSE, 0, 0);

	fontCache[letter].push_back(newRecord);
}

void Font::removeFromCache(const string & text) {
	int letter = numCharInAlphabet(text[0]);
	if (fontCache[letter].size() > 0) {
		for (unsigned int i = 0; i < fontCache[letter].size(); i++) {
			if ((fontCache[letter][i].text == text) && (fontCache[letter][i].fontName == name_)) {
				glDeleteTextures(1, &fontCache[letter][i].texture);
				glDeleteBuffers(1, &fontCache[letter][i].vbo);
				fontCache[letter].erase(fontCache[letter].begin()+i);
				break;
			}
		}
	}
}

void initFont(Shader * newFontShader) {
	fontShader = newFontShader;
	TTF_Init();
}

void quitFont() {
	TTF_Quit();
	fontShader = NULL;
	clearFontCache();
}

void bindFontShader(Shader * newFontShader) {
	fontShader = newFontShader;
}

void clearFontCache() {
	for (int i = 0; i < 27; i++) {
		if (fontCache[i].size() > 0) {
			for (unsigned int j = 0; j < fontCache[i].size(); j++) {
				glDeleteTextures(1, &fontCache[i][j].texture);
				glDeleteBuffers(1, &fontCache[i][j].vbo);
			}
			fontCache[i].clear();
		}
	}
}

}
