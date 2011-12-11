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

typedef unsigned GLuint;

namespace SuperMaximo {

class Object;
class Shader;

class Sprite {
	std::string name_;
	std::vector<GLuint> texture_;
	unsigned frames, framerate_;
	struct spriteRect {
		int x, y;
		unsigned w, h;
		spriteRect(int x, int y, unsigned w, unsigned h) : x(x), y(y), w(w), h(h) {};
	} rect;
	int originX_, originY_;
	GLuint vao, vbo;
	Shader * boundShader_;

public:
	friend class Object;

	Sprite(const std::string & name, const std::string & fileName, int x = 0, int y = 0, int width = 0,
			int height = 0, int frames = 1, unsigned framerate = 1, int originX = 0, int originY = 0);
	~Sprite();

	const std::string & name();

	unsigned frameCount();
	void setFramerate(unsigned newFramerate);
	unsigned framerate();

	void draw(int x, int y, float depth, float rotation = 0.0f, float xScale = 1.0f, float yScale = 1.0f,
			float alpha = 1.0f, unsigned frame = 1, Shader * shaderOverride = NULL);

	void draw(Object & object);

	int width(), height(), originX(), originY();

	GLuint texture(unsigned frame);

	void bindShader(Shader * shader);
	Shader * boundShader();
};

}

#endif /* SPRITE_H_ */
