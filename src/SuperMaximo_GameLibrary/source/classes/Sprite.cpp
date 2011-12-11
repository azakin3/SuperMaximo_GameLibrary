//============================================================================
// Name        : Sprite.cpp
// Author      : Max Foster
// Created on  : 30 May 2011
// Version     : 1.0
// Copyright   : http://creativecommons.org/licenses/by/3.0/
// Description : SuperMaximo GameLibrary Sprite class
//============================================================================

#include <iostream>
#include <vector>
using namespace std;

#include <GL/glew.h>
#include <SDL/SDL_image.h>

#include <SuperMaximo_GameLibrary/Display.h>
#include <SuperMaximo_GameLibrary/classes/Shader.h>
#include <SuperMaximo_GameLibrary/classes/Object.h>
#include <SuperMaximo_GameLibrary/classes/Sprite.h>

namespace SuperMaximo {

Sprite::Sprite(const string & name, const string & fileName, int x, int y, int width,
		int height, int newFrames, unsigned framerate, int originX, int originY) :
		name_(name), frames(newFrames), framerate_(framerate), rect(x, y, width, height), originX_(originX),
		originY_(originY) {

	SDL_Surface * image = IMG_Load(fileName.c_str());
	GLenum textureType;
	if (textureRectangleEnabled()) textureType = GL_TEXTURE_RECTANGLE; else textureType = GL_TEXTURE_2D;

	if (image == NULL) cout << "Could not load image " << fileName << endl; else {
		SDL_SetAlpha(image, 0, 0);
		GLenum textureFormat;
		if (image->format->BytesPerPixel == 4) {
			if (image->format->Rmask == 0x000000ff) textureFormat = GL_RGBA; else textureFormat = GL_BGRA;
		} else {
			if (image->format->Rmask == 0x000000ff) textureFormat = GL_RGB; else textureFormat = GL_BGR;
		}
		if (rect.w < 1) rect.w = image->w;
		if (rect.h < 1) rect.h = image->h;
		SDL_Surface * tempSurface = SDL_CreateRGBSurface(SDL_HWSURFACE, rect.w, rect.h,
				int(image->format->BitsPerPixel), image->format->Rmask, image->format->Gmask, image->format->Bmask,
				image->format->Amask);

		SDL_Rect tempRect;
		tempRect.w = rect.w, tempRect.h = rect.h;
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
			tempRect.x = rect.x+(frame*rect.w);
			tempRect.y = rect.y+(row*rect.h);
			SDL_BlitSurface(image, &tempRect, tempSurface, NULL);
			glBindTexture(textureType, texture_[i]);
			glTexParameteri(textureType, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(textureType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexImage2D(textureType, 0, tempSurface->format->BytesPerPixel, rect.w, rect.h, 0, textureFormat,
					GL_UNSIGNED_BYTE, tempSurface->pixels);
		}
		SDL_FreeSurface(tempSurface);
	}
	boundShader_ = NULL;
	if (vertexArrayObjectSupported()) {
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
	}

	GLfloat vertexArray[] = {
		0.0f, rect.h, 0.0f, 1.0f,
		0.0f, 0.0f, 0.0f, 1.0f,
		rect.w, 0.0f, 0.0f, 1.0f,

		0.0f, rect.h, 0.0f, 1.0f,
		rect.w, rect.h, 0.0f, 1.0f,
		rect.w, 0.0f, 0.0f, 1.0f
	};

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexArray), vertexArray, GL_STATIC_DRAW);
	glVertexAttribPointer(VERTEX_ATTRIBUTE, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(VERTEX_ATTRIBUTE);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	if (vertexArrayObjectSupported()) glBindVertexArray(0);
	for (char i = 0; i < 16; i++) glDisableVertexAttribArray(i);
	glBindTexture(textureType, 0);
}

Sprite::~Sprite() {
	for (unsigned i = 0; i < frames; i++) glDeleteTextures(1, &(texture_[i]));
	glDeleteBuffers(1, &vbo);
	if (vertexArrayObjectSupported()) glDeleteVertexArrays(1, &vao);
}

const string & Sprite::name() {
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
		unsigned frame, Shader * shaderOverride) {
	Shader * shaderToUse;
	if (shaderOverride != NULL) shaderToUse = shaderOverride;
	else if (boundShader_ != NULL) shaderToUse = boundShader_;
	else shaderToUse = SuperMaximo::boundShader();

	if (shaderToUse != NULL) {
		if (frame >= frames) frame = frames-1;
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(textureRectangleEnabled() ? GL_TEXTURE_RECTANGLE : GL_TEXTURE_2D, texture_[frame]);

		pushMatrix();
			translateMatrix(x-originX_, y-originY_, depth);
			translateMatrix(originX_, originY_, 0.0f);
			rotateMatrix(rotation, 0.0f, 0.0f, 1.0f);
			scaleMatrix(xScale, yScale, 0.0f);
			translateMatrix(-originX_, -originY_, 0.0f);

			shaderToUse->use();
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

			glDrawArrays(GL_TRIANGLES, 0, 6);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			if (vertexArrayObjectSupported()) glBindVertexArray(0);
		popMatrix();
	}
}

void Sprite::draw(Object & object) {
	draw(object.x_, object.y_, object.z_, object.zRotation_, object.xScale_, object.yScale_, object.alpha_,
			object.frame_.front(), object.boundShader_);
}

int Sprite::width() {
	return rect.w;
}

int Sprite::height() {
	return rect.h;
}

int Sprite::originX() {
	return originX_;
}

int Sprite::originY() {
	return originY_;
}

GLuint Sprite::texture(unsigned frame) {
	if (frame >= frames) frame = frames-1;
	return texture_[frame];
}

void Sprite::bindShader(Shader * shader) {
	boundShader_ = shader;
}

Shader * Sprite::boundShader() {
	return boundShader_;
}

}
