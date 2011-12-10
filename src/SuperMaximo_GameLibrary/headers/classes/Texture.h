//============================================================================
// Name        : Texture.h
// Author      : Max Foster
// Created on  : 6 Jun 2011
// Version     : 1.0
// Copyright   : http://creativecommons.org/licenses/by/3.0/
// Description : SuperMaximo GameLibrary Texture class
//============================================================================

#ifndef TEXTURE_H_
#define TEXTURE_H_

#include <iostream>
#include <vector>

typedef unsigned GLuint;

namespace SuperMaximo {

enum textureTypeEnum {
	TEXTURE_1D = 0x0DE0,
	TEXTURE_2D = 0x0DE1,
	TEXTURE_3D = 0x806F,
	TEXTURE_RECTANGLE = 0x84F5,
	TEXTURE_CUBE = 0x8513
};

class Texture {
	std::string name_;
	GLuint texture;
	textureTypeEnum type_;
	int width_, height_;

public:
	operator GLuint() const;

	Texture(const std::string & name, textureTypeEnum textureType, const std::string & fileName, ...);
	Texture(const std::string & name, textureTypeEnum textureType, unsigned numLayers, ...);
	Texture(const std::string & name, textureTypeEnum textureType, const std::vector<std::string> & fileNames);
	Texture(const std::string & name, textureTypeEnum textureType, unsigned numLayers, std::string * fileNames);
	~Texture();

	void reload(textureTypeEnum textureType, const std::string & fileName, ...);
	void reload(textureTypeEnum textureType, unsigned numLayers, ...);
	void reload(textureTypeEnum textureType, const std::vector<std::string> & fileNames);
	void reload(textureTypeEnum textureType, unsigned numLayers, std::string * fileNames);

	const std::string & name() const;
	textureTypeEnum type() const;
	int width() const, height() const;
};

}

#endif /* TEXTURE_H_ */
