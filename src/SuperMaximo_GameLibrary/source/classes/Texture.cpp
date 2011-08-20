//============================================================================
// Name        : Texture.cpp
// Author      : Max Foster
// Created on  : 6 Jun 2011
// Version     : 1.0
// Copyright   : http://creativecommons.org/licenses/by/3.0/
// Description : SuperMaximo GameLibrary Texture class
//============================================================================

#define GL3_PROTOTYPES 1
#include <GL3/gl3.h>

#include <iostream>
#include <vector>
using namespace std;
#include <SDL/SDL_image.h>
#include "../../headers/classes/Texture.h"
#include "../../headers/Utils.h"
#include "../../headers/Display.h"
using namespace SuperMaximo;

vector<Texture*> allTextures[27];

namespace SuperMaximo {

Texture::operator GLuint() {
	return texture;
}

Texture::Texture(string newName, textureTypeEnum textureType, string fileName, ...) {
	name_ = newName;
	vector<string> fileNames;
	fileNames.push_back(fileName);
	va_list files;
	va_start(files, fileName);
	for (short i = 0; i < 4; i++) {
		char * file = va_arg(files, char *);
		fileNames.push_back(file);
	}
	reload(textureType, fileNames);
}

Texture::Texture(string newName, textureTypeEnum textureType, unsigned numLayers, ...) {
	name_ = newName;
	vector<string> fileNames;
	va_list files;
	va_start(files, numLayers);
	for (unsigned i = 0; i < numLayers; i++) {
		char * file = va_arg(files, char *);
		fileNames.push_back(file);
	}
	reload(textureType, fileNames);
}

Texture::Texture(string newName, textureTypeEnum textureType, vector<string> fileNames) {
	name_ = newName;
	reload(textureType, fileNames);
}

Texture::Texture(string newName, textureTypeEnum textureType, unsigned numLayers, string * fileNames) {
	name_ = newName;
	reload(textureType, numLayers, fileNames);
}

Texture::~Texture() {
	glDeleteTextures(1, &texture);
}

void Texture::reload(textureTypeEnum textureType, string fileName, ...) {
	type_ = textureType;
	if (textureType == TEXTURE_3D) cout << "Cannot create a 3D texture with the arguments given" << endl; else {
		SDL_Surface * image = IMG_Load(fileName.c_str());
		if (image == NULL) cout << "Could not load image " << fileName << endl; else {
			GLenum textureFormat;
			if (image->format->BytesPerPixel == 4) {
				if (image->format->Rmask == 0x000000ff) textureFormat = GL_RGBA; else textureFormat = GL_BGRA;
			} else {
				if (image->format->Rmask == 0x000000ff) textureFormat = GL_RGB; else textureFormat = GL_BGR;
			}
			glGenTextures(1, &texture);
			glBindTexture(textureType, texture);
			glTexParameteri(textureType, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(textureType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			if (textureType != TEXTURE_CUBE) {
				glTexImage2D(textureType, 0, image->format->BytesPerPixel, image->w, image->h, 0, textureFormat,
						GL_UNSIGNED_BYTE, image->pixels);
				SDL_FreeSurface(image);
			} else {
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, image->format->BytesPerPixel, image->w, image->h, 0,
						textureFormat, GL_UNSIGNED_BYTE, image->pixels);
				SDL_FreeSurface(image);

				GLenum sides[5] = {GL_TEXTURE_CUBE_MAP_NEGATIVE_X, GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
						GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z};
				va_list files;
				va_start(files, fileName);
				for (short i = 0; i < 4; i++) {
					char * file = va_arg(files, char *);
					image = IMG_Load(file);
					if (image == NULL) cout << "Could not load image " << file << endl; else {
						glTexImage2D(sides[i], 0, image->format->BytesPerPixel, image->w, image->h, 0, textureFormat,
								GL_UNSIGNED_BYTE, image->pixels);
						SDL_FreeSurface(image);
					}
				}
				va_end(files);
			}
			glBindTexture(textureType, 0);
		}
	}
}

void Texture::reload(textureTypeEnum textureType, unsigned numLayers, ...) {
	type_ = textureType;
	if (textureType == TEXTURE_3D) {
		glGenTextures(1, &texture);
		if (texture2dArrayDisabled()) {
			glBindTexture(GL_TEXTURE_2D, texture);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		} else {
			glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}

		bool initialised = false;
		va_list files;
		va_start(files, numLayers);
		for (unsigned i = 0; i < numLayers; i++) {
			char * file = va_arg(files, char *);
			SDL_Surface * image = IMG_Load(file);
			if (image == NULL) cout << "Could not load image " << file << endl; else {
				GLenum textureFormat;
				if (image->format->BytesPerPixel == 4) {
					if (image->format->Rmask == 0x000000ff) textureFormat = GL_RGBA; else textureFormat = GL_BGRA;
				} else {
					if (image->format->Rmask == 0x000000ff) textureFormat = GL_RGB; else textureFormat = GL_BGR;
				}
				if (!initialised) {
					if (texture2dArrayDisabled()) glTexImage2D(GL_TEXTURE_2D, 0, image->format->BytesPerPixel,
							image->w*numLayers, image->h, 0, textureFormat, GL_UNSIGNED_BYTE, NULL);
					else glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, image->format->BytesPerPixel, image->w, image->h,
							numLayers, 0, textureFormat, GL_UNSIGNED_BYTE, NULL);
					initialised = true;
				}
				if (texture2dArrayDisabled()) glTexSubImage2D(GL_TEXTURE_2D, 0, image->w*i, 0, image->w, image->h,
						textureFormat, GL_UNSIGNED_BYTE, image->pixels);
				else glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, image->w, image->h, 1, textureFormat,
						GL_UNSIGNED_BYTE, image->pixels);
				SDL_FreeSurface(image);
			}
		}
		va_end(files);
		glBindTexture(textureType, 0);
	} else cout << "Wrong type of texture specified" << endl;
}

void Texture::reload(textureTypeEnum textureType, vector<string> fileNames) {
	type_ = textureType;
	if (textureType == TEXTURE_3D) {
		glGenTextures(1, &texture);
		if (texture2dArrayDisabled()) {
			glBindTexture(GL_TEXTURE_2D, texture);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		} else {
			glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}

		bool initialised = false;
		for (unsigned i = 0; i < fileNames.size(); i++) {
			SDL_Surface * image = IMG_Load(fileNames[i].c_str());
			if (image == NULL) cout << "Could not load image " << fileNames[i] << endl; else {
				GLenum textureFormat;
				if (image->format->BytesPerPixel == 4) {
					if (image->format->Rmask == 0x000000ff) textureFormat = GL_RGBA; else textureFormat = GL_BGRA;
				} else {
					if (image->format->Rmask == 0x000000ff) textureFormat = GL_RGB; else textureFormat = GL_BGR;
				}
				if (!initialised) {
					if (texture2dArrayDisabled()) glTexImage2D(GL_TEXTURE_2D, 0, image->format->BytesPerPixel,
							image->w*fileNames.size(), image->h, 0, textureFormat, GL_UNSIGNED_BYTE, NULL);
					else glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, image->format->BytesPerPixel, image->w, image->h,
							fileNames.size(), 0, textureFormat, GL_UNSIGNED_BYTE, NULL);
					initialised = true;
				}
				if (texture2dArrayDisabled()) glTexSubImage2D(GL_TEXTURE_2D, 0, image->w*i, 0, image->w, image->h,
						textureFormat, GL_UNSIGNED_BYTE, image->pixels);
				else glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, image->w, image->h, 1, textureFormat,
						GL_UNSIGNED_BYTE, image->pixels);
				SDL_FreeSurface(image);
			}
		}
		glBindTexture(textureType, 0);
	} else {
		SDL_Surface * image = IMG_Load(fileNames[0].c_str());
		if (image == NULL) cout << "Could not load image " << fileNames[0] << endl; else {
			GLenum textureFormat;
			if (image->format->BytesPerPixel == 4) {
				if (image->format->Rmask == 0x000000ff) textureFormat = GL_RGBA; else textureFormat = GL_BGRA;
			} else {
				if (image->format->Rmask == 0x000000ff) textureFormat = GL_RGB; else textureFormat = GL_BGR;
			}
			glGenTextures(1, &texture);
			glBindTexture(textureType, texture);
			glTexParameteri(textureType, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(textureType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			if (textureType != TEXTURE_CUBE) {
				glTexImage2D(textureType, 0, image->format->BytesPerPixel, image->w, image->h, 0, textureFormat,
						GL_UNSIGNED_BYTE, image->pixels);
				SDL_FreeSurface(image);
			} else {
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, image->format->BytesPerPixel, image->w, image->h, 0,
						textureFormat, GL_UNSIGNED_BYTE, image->pixels);
				SDL_FreeSurface(image);

				GLenum sides[5] = {GL_TEXTURE_CUBE_MAP_NEGATIVE_X, GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
						GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z};
				for (short i = 0; i < 4; i++) {
					image = IMG_Load(fileNames[i+1].c_str());
					if (image == NULL) cout << "Could not load image " << fileNames[i+1] << endl; else {
						glTexImage2D(sides[i], 0, image->format->BytesPerPixel, image->w, image->h, 0, textureFormat,
								GL_UNSIGNED_BYTE, image->pixels);
						SDL_FreeSurface(image);
					}
				}
			}
			glBindTexture(textureType, 0);
		}
	}
}

void Texture::reload(textureTypeEnum textureType, unsigned numLayers, string * fileNames) {
	type_ = textureType;
	if (textureType == TEXTURE_3D) {
		glGenTextures(1, &texture);
		if (texture2dArrayDisabled()) {
			glBindTexture(GL_TEXTURE_2D, texture);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		} else {
			glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}

		bool initialised = false;
		for (unsigned i = 0; i < numLayers; i++) {
			SDL_Surface * image = IMG_Load(fileNames[i].c_str());
			if (image == NULL) cout << "Could not load image " << fileNames[i] << endl; else {
				GLenum textureFormat;
				if (image->format->BytesPerPixel == 4) {
					if (image->format->Rmask == 0x000000ff) textureFormat = GL_RGBA; else textureFormat = GL_BGRA;
				} else {
					if (image->format->Rmask == 0x000000ff) textureFormat = GL_RGB; else textureFormat = GL_BGR;
				}
				if (!initialised) {
					if (texture2dArrayDisabled()) glTexImage2D(GL_TEXTURE_2D, 0, image->format->BytesPerPixel,
							image->w*numLayers, image->h, 0, textureFormat, GL_UNSIGNED_BYTE, NULL);
					else glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, image->format->BytesPerPixel, image->w, image->h,
							numLayers, 0, textureFormat, GL_UNSIGNED_BYTE, NULL);
					initialised = true;
				}
				if (texture2dArrayDisabled()) glTexSubImage2D(GL_TEXTURE_2D, 0, image->w*i, 0, image->w, image->h,
						textureFormat, GL_UNSIGNED_BYTE, image->pixels);
				else glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, image->w, image->h, 1, textureFormat,
						GL_UNSIGNED_BYTE, image->pixels);
				SDL_FreeSurface(image);
			}
		}
		glBindTexture(textureType, 0);
	} else {
		SDL_Surface * image = IMG_Load(fileNames[0].c_str());
		if (image == NULL) cout << "Could not load image " << fileNames[0] << endl; else {
			GLenum textureFormat;
			if (image->format->BytesPerPixel == 4) {
				if (image->format->Rmask == 0x000000ff) textureFormat = GL_RGBA; else textureFormat = GL_BGRA;
			} else {
				if (image->format->Rmask == 0x000000ff) textureFormat = GL_RGB; else textureFormat = GL_BGR;
			}
			glGenTextures(1, &texture);
			glBindTexture(textureType, texture);
			glTexParameteri(textureType, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(textureType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			if (textureType != TEXTURE_CUBE) {
				glTexImage2D(textureType, 0, image->format->BytesPerPixel, image->w, image->h, 0, textureFormat,
						GL_UNSIGNED_BYTE, image->pixels);
				SDL_FreeSurface(image);
			} else {
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, image->format->BytesPerPixel, image->w, image->h, 0,
						textureFormat, GL_UNSIGNED_BYTE, image->pixels);
				SDL_FreeSurface(image);

				GLenum sides[5] = {GL_TEXTURE_CUBE_MAP_NEGATIVE_X, GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
						GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z};
				for (short i = 0; i < 4; i++) {
					image = IMG_Load(fileNames[i+1].c_str());
					if (image == NULL) cout << "Could not load image " << fileNames[i+1] << endl; else {
						glTexImage2D(sides[i], 0, image->format->BytesPerPixel, image->w, image->h, 0, textureFormat,
								GL_UNSIGNED_BYTE, image->pixels);
						SDL_FreeSurface(image);
					}
				}
			}
			glBindTexture(textureType, 0);
		}
	}
}

string Texture::name() {
	return name_;
}

textureTypeEnum Texture::type() {
	return type_;
}

Texture * texture(std::string searchName) {
	int letter = numCharInAlphabet(searchName[0]);
	Texture * returnTexture = NULL;
	if (allTextures[letter].size() > 0) {
		for (unsigned int i = 0; i < allTextures[letter].size(); i++) {
			if (allTextures[letter][i]->name() == searchName) returnTexture = allTextures[letter][i];
		}
	}
	return returnTexture;
}

Texture * addTexture(std::string newName, textureTypeEnum textureType, std::string fileName, ...) {
	int letter = numCharInAlphabet(newName[0]);
	vector<string> fileNames;
	fileNames.push_back(fileName);
	va_list files;
	va_start(files, fileName);
	for (short i = 0; i < 4; i++) {
		char * file = va_arg(files, char *);
		fileNames.push_back(file);
	}
	Texture * newTexture = new Texture(newName, textureType, fileNames);
	allTextures[letter].push_back(newTexture);
	return newTexture;
}

Texture * addTexture(std::string newName, textureTypeEnum textureType, unsigned numLayers, ...) {
	int letter = numCharInAlphabet(newName[0]);
	vector<string> fileNames;
	va_list files;
	va_start(files, numLayers);
	for (unsigned i = 0; i < numLayers; i++) {
		char * file = va_arg(files, char *);
		fileNames.push_back(file);
	}
	Texture * newTexture = new Texture(newName, textureType, fileNames);
	allTextures[letter].push_back(newTexture);
	return newTexture;
}

void destroyTexture(std::string searchName) {
	int letter = numCharInAlphabet(searchName[0]);
	if (allTextures[letter].size() > 0) {
		for (unsigned int i = 0; i < allTextures[letter].size(); i++) {
			if (allTextures[letter][i]->name() == searchName) {
				delete allTextures[letter][i];
				allTextures[letter].erase(allTextures[letter].begin()+i);
				break;
			}
		}
	}
}

void destroyAllTextures() {
	for (int i = 0; i < 27; i++) {
		if (allTextures[i].size() > 0) {
			for (unsigned int j = 0; j < allTextures[i].size(); j++) delete allTextures[i][j];
			allTextures[i].clear();
		}
	}
}

}
