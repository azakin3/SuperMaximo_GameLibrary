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
#include <GL/gl.h>

namespace SuperMaximo {

enum textureTypeEnum {
	TEXTURE_1D = GL_TEXTURE_1D,
	TEXTURE_2D = GL_TEXTURE_2D,
	TEXTURE_3D = GL_TEXTURE_3D,
	TEXTURE_RECTANGLE = GL_TEXTURE_RECTANGLE,
	TEXTURE_CUBE = GL_TEXTURE_CUBE_MAP
};

class Texture {
	std::string name_;
	GLuint texture;
	textureTypeEnum type_;
public:
	operator GLuint();
	Texture(std::string newName, textureTypeEnum textureType, std::string fileName, ...);
	Texture(std::string newName, textureTypeEnum textureType, unsigned numLayers, ...);
	Texture(std::string newName, textureTypeEnum textureType, std::vector<std::string> fileNames);
	~Texture();
	void reload(textureTypeEnum textureType, std::string fileName, ...);
	void reload(textureTypeEnum textureType, unsigned numLayers, ...);
	void reload(textureTypeEnum textureType, std::vector<std::string> fileNames);
	std::string name();
	textureTypeEnum type();
};

Texture * texture(std::string searchName);
Texture * addTexture(std::string newName, textureTypeEnum textureType, std::string fileName, ...);
Texture * addTexture(std::string newName, textureTypeEnum textureType, unsigned numLayers, ...);
void destroyTexture(std::string searchName);
void destroyAllTextures();

}

#endif /* TEXTURE_H_ */
