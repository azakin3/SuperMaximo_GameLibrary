//============================================================================
// Name        : Sprite.h
// Author      : Max Foster
// Created on  : 30 May 2011
// Version     : 1.0
// Copyright   : http://creativecommons.org/licenses/by/3.0/
// Description : SuperMaximo GameLibrary Sprite class
//============================================================================

#ifndef SPRITE_H_
#define SPRITE_H_

#include <iostream>
#include <vector>
#include <SDL/SDL_image.h>
#include <GL/glew.h>

namespace SuperMaximo {

class Object;
class Shader;

typedef void (*customDrawFunctionType)(void*, Shader*, void*);

struct spriteDrawParams {
	int x, y;
	unsigned frame;
	float depth, rotation, xScale, yScale, alpha;
};

class Sprite {
	SDL_Surface * image;
	std::vector<GLuint> texture_;
	unsigned frames, vertices_, framerate_;
	int originX, originY;
	std::string name_;
	SDL_Rect rect;
	GLuint vao, vbo;
	Shader * boundShader_;
	customDrawFunctionType customDrawFunction;
	void initBuffer();
public:
	friend class Object;
	Sprite(std::string newName, std::string fileName, int imageX, int imageY, int imageWidth, int imageHeight,
			int aniFrames = 1, unsigned framerate = 1, int newOriginX = -1, int newOriginY = -1,
			void (*customBufferFunction)(GLuint*, Sprite*, void*) = NULL, void * customData = NULL);

	Sprite(std::string newName, SDL_Surface * surface, int imageX, int imageY, int imageWidth, int imageHeight,
			int aniFrames = 1, unsigned framerate = 1, int newOriginX = -1, int newOriginY = -1,
			void (*customBufferFunction)(GLuint*, Sprite*, void*) = NULL, void * customData = NULL);
	~Sprite();

	std::string name();

	unsigned frameCount();
	void setFramerate(unsigned newFramerate);
	unsigned framerate();

	void draw(int x, int y, float depth, float rotation = 0.0f, float xScale = 1.0f, float yScale = 1.0f,
			float alpha = 1.0f, unsigned frame = 1, Shader * shaderOverride = NULL,
			customDrawFunctionType customDrawFunctionOverride = NULL);

	void draw(Object * object);
	void defaultDraw(Shader * shaderToUse, spriteDrawParams * params);

	SDL_Surface * drawToSurface(float rotation = 0.0f, float xScale = 1.0f, float yScale = 1.0f,
			float alpha = 1.0f, unsigned frame = 1);

	int width();
	int height();

	SDL_Surface * surface();
	GLuint texture(unsigned frame);

	unsigned vertices();

	void bindShader(Shader * shader);
	Shader * boundShader();

	void bindCustomDrawFunction(customDrawFunctionType newCustomDrawFunction);
	customDrawFunctionType boundCustomDrawFunction();
};

Sprite * sprite(std::string searchName);

Sprite * addSprite(std::string newName, std::string fileName, int imageX, int imageY, int imageWidth,
		int imageHeight, int aniFrames = 1, unsigned framerate = 1, int newOriginX = 1, int newOriginY = 1,
		void (*customBufferFunction)(GLuint*, Sprite*, void*) = NULL, void * customData = NULL);

Sprite * addSprite(std::string newName, SDL_Surface * surface, int imageX, int imageY, int imageWidth,
		int imageHeight, int aniFrames = 1, unsigned framerate = 1, int newOriginX = 1, int newOriginY = 1,
		void (*customBufferFunction)(GLuint*, Sprite*, void*) = NULL, void * customData = NULL);

void destroySprite(std::string searchName);

void destroyAllSprites();

}

#endif /* SPRITE_H_ */
