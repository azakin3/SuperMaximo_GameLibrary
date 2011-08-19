//============================================================================
// Name        : Sprite.cpp
// Author      : Max Foster
// Created on  : 30 May 2011
// Version     : 1.0
// Copyright   : http://creativecommons.org/licenses/by/3.0/
// Description : SuperMaximo GameLibrary Sprite class
//============================================================================

#define GL3_PROTOTYPES 1
#include <GL3/gl3.h>

#include <iostream>
#include <vector>
using namespace std;
#include <SDL/SDL_image.h>
#include <SDL/SDL_rotozoom.h>
#include "../../headers/classes/Sprite.h"
#include "../../headers/classes/Object.h"
#include "../../headers/classes/Shader.h"
#include "../../headers/Display.h"
#include "../../headers/Utils.h"
using namespace SuperMaximo;

vector<Sprite*> allSprites[27];

namespace SuperMaximo {

Sprite::Sprite(string newName, string fileName, int imageX, int imageY, int imageWidth, int imageHeight,
		int aniFrames, unsigned framerate, int newOriginX, int newOriginY,
		void (*customBufferFunction)(GLuint*, Sprite*, void*), void * customData) {
	name_ = newName;
	frames = aniFrames;
	framerate_ = framerate;
	rect.x = imageX;
	rect.y = imageY;
	rect.w = imageWidth;
	rect.h = imageHeight;
	originX = newOriginX;
	originY = newOriginY;
	customDrawFunction = NULL;
	image = IMG_Load(fileName.c_str());
	if (image == NULL) cout << "Could not load image " << fileName << endl; else {
		SDL_SetAlpha(image, 0, 0);
		GLenum textureFormat;
		if (image->format->BytesPerPixel == 4) {
			if (image->format->Rmask == 0x000000ff) textureFormat = GL_RGBA; else textureFormat = GL_BGRA;
		} else {
			if (image->format->Rmask == 0x000000ff) textureFormat = GL_RGB; else textureFormat = GL_BGR;
		}
		SDL_Surface * tempSurface = SDL_CreateRGBSurface(SDL_HWSURFACE, rect.w, rect.h,
				int(image->format->BitsPerPixel), image->format->Rmask, image->format->Gmask, image->format->Bmask,
				image->format->Amask);

		SDL_Rect tempRect = rect;
		for (unsigned i = 0; i < frames; i++) {
			texture_.push_back(0);
			glGenTextures(1, &(texture_[i]));
			int frame = i, row = 0;
			int numFrames = div(image->w, rect.w).quot;
			if (numFrames > 0) {
				while (frame-(row*numFrames) >= numFrames) {
					row++;
					frame -= numFrames;
				}
			}
			tempRect.x = frame*rect.w;
			tempRect.y = row*rect.h;
			SDL_BlitSurface(image, &tempRect, tempSurface, NULL);
			glBindTexture(GL_TEXTURE_RECTANGLE, texture_[i]);
			glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexImage2D(GL_TEXTURE_RECTANGLE, 0, tempSurface->format->BytesPerPixel, rect.w, rect.h, 0, textureFormat,
					GL_UNSIGNED_BYTE, tempSurface->pixels);
		}
		SDL_FreeSurface(tempSurface);
	}
	boundShader_ = NULL;
	vertices_ = 6;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	if (customBufferFunction == NULL) initBuffer(); else (*customBufferFunction)(&vbo, this, customData);
	glBindVertexArray(0);
	for (char i = 0; i < 16; i++) glDisableVertexAttribArray(i);
	glBindTexture(GL_TEXTURE_RECTANGLE, 0);
}

Sprite::Sprite(string newName, SDL_Surface * surface, int imageX, int imageY, int imageWidth, int imageHeight,
		int aniFrames, unsigned framerate, int newOriginX, int newOriginY,
		void (*customBufferFunction)(GLuint*, Sprite*, void*), void * customData) {
	name_ = newName;
	frames = aniFrames;
	framerate_ = framerate;
	rect.x = imageX;
	rect.y = imageY;
	rect.w = imageWidth;
	rect.h = imageHeight;
	originX = newOriginX;
	originY = newOriginY;
	customDrawFunction = NULL;
	image = surface;
	GLenum textureFormat;
	if (image->format->BytesPerPixel == 4) {
		if (image->format->Rmask == 0x000000ff) textureFormat = GL_RGBA; else textureFormat = GL_BGRA;
	} else {
		if (image->format->Rmask == 0x000000ff) textureFormat = GL_RGB; else textureFormat = GL_BGR;
	}
	SDL_Surface * tempSurface = SDL_CreateRGBSurface(SDL_HWSURFACE, rect.w, rect.h, 32, 0, 0, 0, 0);
	SDL_Rect tempRect = rect;
	for (unsigned i = 0; i < frames; i++) {
		int frame = i, row = 0;
		int numFrames = div(image->w, rect.w).quot;
		if (numFrames > 0) {
			while (frame-(row*numFrames) >= numFrames) {
				row++;
				frame -= numFrames;
			}
		}
		tempRect.x = frame*rect.w;
		tempRect.y = row*rect.h;
		SDL_BlitSurface(image, &tempRect, tempSurface, NULL);
		glBindTexture(GL_TEXTURE_RECTANGLE, texture_[i]);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_RECTANGLE, 0, image->format->BytesPerPixel, rect.w, rect.h, 0, textureFormat,
				GL_UNSIGNED_BYTE, tempSurface->pixels);
	}
	SDL_FreeSurface(tempSurface);
	boundShader_ = NULL;
	vertices_ = 6;

	if (vertexArrayObjectSupported()) {
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
	}
	if (customBufferFunction == NULL) initBuffer(); else (*customBufferFunction)(&vbo, this, customData);
	if (vertexArrayObjectSupported()) glBindVertexArray(0);
	for (char i = 0; i < 16; i++) glDisableVertexAttribArray(i);
	glBindTexture(GL_TEXTURE_RECTANGLE, 0);
}

Sprite::~Sprite() {
	if (image != NULL) {
		SDL_FreeSurface(image);
		for (unsigned i = 0; i < frames; i++) glDeleteTextures(1, &(texture_[i]));
	}
	glDeleteBuffers(1, &vbo);
	if (vertexArrayObjectSupported()) glDeleteVertexArrays(1, &vao);
}

void Sprite::initBuffer() {
	GLfloat vertexArray[] = {
		0.0f, rect.h, 0.0f, 1.0f,
		0.0f, 0.0f, 0.0f, 1.0f,
		rect.w, 0.0f, 0.0f, 1.0f,

		0.0f, rect.h, 0.0f, 1.0f,
		rect.w, rect.h, 0.0f, 1.0f,
		rect.w, 0.0f, 0.0f, 1.0f};

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexArray), vertexArray, GL_STATIC_DRAW);
	glVertexAttribPointer(VERTEX_ATTRIBUTE, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(VERTEX_ATTRIBUTE);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

string Sprite::name() {
	return name_;
}

unsigned Sprite::frameCount() {
	return frames;
}

void Sprite::setFramerate(unsigned newFramerate) {
	framerate_ = newFramerate;
	if (framerate_ == 0) framerate_ = 1;
}

unsigned Sprite::framerate() {
	return framerate_;
}

void Sprite::draw(int x, int y, float depth, float rotation, float xScale, float yScale, float alpha,
		unsigned frame, Shader * shaderOverride, customDrawFunctionType customDrawFunctionOverride) {
	Shader * shaderToUse;
	if (shaderOverride != NULL) shaderToUse = shaderOverride;
		else if (boundShader_ != NULL) shaderToUse = boundShader_;
			else shaderToUse = ::boundShader();

	customDrawFunctionType drawFunctionToUse;
	if (customDrawFunctionOverride != NULL) drawFunctionToUse = customDrawFunctionOverride;
		else if (customDrawFunction != NULL) drawFunctionToUse = customDrawFunction;
			else drawFunctionToUse = ::boundCustomDrawFunction();

	spriteDrawParams params;
	params.x = x;
	params.y = y;
	params.depth = depth;
	params.rotation = rotation;
	params.xScale = xScale;
	params.yScale = yScale;
	params.alpha = alpha;
	params.frame = frame;

	if (drawFunctionToUse != NULL) drawFunctionToUse(this, shaderToUse, &params);
		else defaultDraw(shaderToUse, &params);

	if (::boundShader() != NULL) glUseProgram(::boundShader()->program_); else glUseProgram(0);
}

void Sprite::draw(Object * object) {
	draw(object->x_, object->y_, object->z_, object->zRotation_, object->xScale_, object->yScale_, object->alpha_,
			object->frame_.front(), object->boundShader_, object->customDrawFunction);
}

void Sprite::defaultDraw(Shader * shaderToUse, spriteDrawParams * params) {
	if (shaderToUse != NULL) {
		while (params->frame >= frames) params->frame--;
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_RECTANGLE, texture_[params->frame-1]);
		pushMatrix();
			translateMatrix(params->x-originX, params->y-originY, params->depth);
			translateMatrix(originX, originY, 0.0f);
			rotateMatrix(params->rotation, 0.0f, 0.0f, 1.0f);
			scaleMatrix(params->xScale, params->yScale, 0.0f);
			translateMatrix(-originX, -originY, 0.0f);

			glUseProgram(shaderToUse->program_);

			shaderToUse->setUniform16(MODELVIEW_LOCATION, getMatrix(MODELVIEW_MATRIX));
			shaderToUse->setUniform16(PROJECTION_LOCATION, getMatrix(PROJECTION_MATRIX));
			shaderToUse->setUniform1(TEXSAMPLER_LOCATION, 0);

			if (vertexArrayObjectSupported()) {
				glBindVertexArray(vao);
				glBindBuffer(GL_ARRAY_BUFFER, vbo);
			} else {
				glBindBuffer(GL_ARRAY_BUFFER, vbo);
				glVertexAttribPointer(VERTEX_ATTRIBUTE, 4, GL_FLOAT, GL_FALSE, 0, 0);
				glEnableVertexAttribArray(VERTEX_ATTRIBUTE);
			}

			glDrawArrays(GL_TRIANGLES, 0, vertices_);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			if (vertexArrayObjectSupported()) glBindVertexArray(0);
		popMatrix();
	}
}

SDL_Surface * Sprite::drawToSurface(float rotation, float xScale, float yScale, float alpha, unsigned frame) {
	while (frame > frames) frame--;
	unsigned numFrames = div(image->w, rect.w).quot;
	int row = 0;
	if (numFrames > 0) {
		while (frame-(row*numFrames) > numFrames) {
			row++;
			frame -= numFrames;
		}
	}
	SDL_Rect toBlitSrcRect;
	toBlitSrcRect.w = rect.w;
	toBlitSrcRect.h = rect.h;
	toBlitSrcRect.x = (frame-1)*rect.w;
	toBlitSrcRect.y = rect.h*row;

	SDL_Surface * preBlit = SDL_CreateRGBSurface(SDL_HWSURFACE, toBlitSrcRect.w, toBlitSrcRect.h, 32, 0, 0, 0, 0);
	SDL_BlitSurface(image, &toBlitSrcRect, preBlit, NULL);
	SDL_Surface * newSurface = rotozoomSurfaceXY(preBlit, rotation, xScale, yScale, 0);
	if (alpha < 1.0f) SDL_SetAlpha(newSurface, SDL_SRCALPHA, alpha*255);
	SDL_SetColorKey(newSurface, SDL_SRCCOLORKEY, 0x000000);
	SDL_FreeSurface(preBlit);
	return newSurface;
}

int Sprite::width() {
	return rect.w;
}

int Sprite::height() {
	return rect.h;
}

SDL_Surface * Sprite::surface() {
	return image;
}

GLuint Sprite::texture(unsigned frame) {
	if (frame >= frames) frame = frames-1;
	return texture_[frame];
}

unsigned Sprite::vertices() {
	return vertices_;
}

void Sprite::bindShader(Shader * shader) {
	boundShader_ = shader;
}

Shader * Sprite::boundShader() {
	return boundShader_;
}

void Sprite::bindCustomDrawFunction(customDrawFunctionType newCustomDrawFunction) {
	customDrawFunction = newCustomDrawFunction;
}

customDrawFunctionType Sprite::boundCustomDrawFunction() {
	return customDrawFunction;
}

Sprite * sprite(string searchName) {
	int letter = numCharInAlphabet(searchName[0]);
	Sprite * returnSprite = NULL;
	if (allSprites[letter].size() > 0) {
		for (unsigned int i = 0; i < allSprites[letter].size(); i++) {
			if (allSprites[letter][i]->name() == searchName) {
				returnSprite = allSprites[letter][i];
				break;
			}
		}
	}
	return returnSprite;
}

Sprite * addSprite(string newName, string fileName, int imageX, int imageY, int imageWidth, int imageHeight,
		int aniFrames, unsigned framerate, int newOriginX, int newOriginY,
		void (*customBufferFunction)(GLuint*, Sprite*, void*), void * customData) {

	int letter = numCharInAlphabet(newName[0]);
	Sprite * newSprite = new Sprite(newName, fileName, imageX, imageY, imageWidth, imageHeight, aniFrames, framerate,
			newOriginX, newOriginY, customBufferFunction, customData);
	allSprites[letter].push_back(newSprite);
	return newSprite;
}

Sprite * addSprite(string newName, SDL_Surface * surface, int imageX, int imageY, int imageWidth, int imageHeight,
		int aniFrames, unsigned framerate, int newOriginX, int newOriginY,
		void (*customBufferFunction)(GLuint*, Sprite*, void*), void * customData) {

	int letter = numCharInAlphabet(newName[0]);
	Sprite * newSprite = new Sprite(newName, surface, imageX, imageY, imageWidth, imageHeight, aniFrames, framerate,
			newOriginX, newOriginY, customBufferFunction, customData);
	allSprites[letter].push_back(newSprite);
	return newSprite;
}

void destroySprite(string searchName) {
	int letter = numCharInAlphabet(searchName[0]);
	if (allSprites[letter].size() > 0) {
		for (unsigned int i = 0; i < allSprites[letter].size(); i++) {
			if (allSprites[letter][i]->name() == searchName) {
				delete allSprites[letter][i];
				allSprites[letter].erase(allSprites[letter].begin()+i);
				break;
			}
		}
	}
}

void destroyAllSprites() {
	for (int i = 0; i < 27; i++) {
		if (allSprites[i].size() > 0) {
			for (unsigned int j = 0; j < allSprites[i].size(); j++) delete allSprites[i][j];
			allSprites[i].clear();
		}
	}
}

}
