//============================================================================
// Name        : Shader.h
// Author      : Max Foster
// Created on  : 31 May 2011
// Version     : 1.0
// Copyright   : http://creativecommons.org/licenses/by/3.0/
// Description : SuperMaximo GameLibrary OpenGL GLSL Shader class
//============================================================================

#ifndef SHADER_H_
#define SHADER_H_

#include <GL/glew.h>
#include <iostream>
#include <vector>

namespace SuperMaximo {

enum shaderAttributeEnum {
	VERTEX_ATTRIBUTE = 0,
	NORMAL_ATTRIBUTE,
	COLOR0_ATTRIBUTE,
	COLOR1_ATTRIBUTE,
	COLOR2_ATTRIBUTE,
	TEXTURE0_ATTRIBUTE,
	EXTRA0_ATTRIBUTE,
	EXTRA1_ATTRIBUTE,
	EXTRA2_ATTRIBUTE,
	EXTRA3_ATTRIBUTE,
	EXTRA4_ATTRIBUTE
};

enum shaderLocationEnum {
	MODELVIEW_LOCATION = 0,
	PROJECTION_LOCATION,
	TEXSAMPLER_LOCATION,
	EXTRA0_LOCATION,
	EXTRA1_LOCATION,
	EXTRA2_LOCATION,
	EXTRA3_LOCATION,
	EXTRA4_LOCATION,
	EXTRA5_LOCATION,
	EXTRA6_LOCATION,
	EXTRA7_LOCATION,
	EXTRA8_LOCATION,
	EXTRA9_LOCATION,
	TEXCOMPAT_LOCATION,
	SHADER_LOCATION_ENUM_COUNT
};

class Sprite;
class Model;
class Font;

class Shader {
	GLuint program_;
	GLint uniformLocation_[SHADER_LOCATION_ENUM_COUNT];
	std::string name_;
public:
	friend class Sprite;
	friend class Model;
	friend class Font;
	Shader(std::string newName, std::string vertexShaderFile, std::string fragmentShaderFile, ...);
	Shader(std::string newName, std::string vertexShaderFile, std::string fragmentShaderFile, std::vector<int> enums,
			std::vector<char *>attributeNames);
	Shader(std::string newName, std::string vertexShaderFile, std::string fragmentShaderFile, unsigned count,
			int * enums, const char ** attributeNames);
	~Shader();
	std::string name();
	void bind();
	void use();
	GLuint program();
	GLint setUniformLocation(shaderLocationEnum dstLocation, std::string locationName);
	GLint uniformLocation(shaderLocationEnum location);

	void setUniform1(shaderLocationEnum location, GLfloat * data, unsigned count = 1);
	void setUniform2(shaderLocationEnum location, GLfloat * data, unsigned count = 1);
	void setUniform3(shaderLocationEnum location, GLfloat * data, unsigned count = 1);
	void setUniform4(shaderLocationEnum location, GLfloat * data, unsigned count = 1);

	void setUniform9(shaderLocationEnum location, GLfloat * data, unsigned count = 1);
	void setUniform16(shaderLocationEnum location, GLfloat * data, unsigned count = 1);

	void setUniform1(shaderLocationEnum location, int * data, unsigned count = 1);
	void setUniform2(shaderLocationEnum location, int * data, unsigned count = 1);
	void setUniform3(shaderLocationEnum location, int * data, unsigned count = 1);
	void setUniform4(shaderLocationEnum location, int * data, unsigned count = 1);

	void setUniform1(shaderLocationEnum location, GLfloat data);
	void setUniform2(shaderLocationEnum location, GLfloat data1, GLfloat data2);
	void setUniform3(shaderLocationEnum location, GLfloat data1, GLfloat data2, GLfloat data3);
	void setUniform4(shaderLocationEnum location, GLfloat data1, GLfloat data2, GLfloat data3, GLfloat data4);

	void setUniform1(shaderLocationEnum location, int data);
	void setUniform2(shaderLocationEnum location, int data1, int data2);
	void setUniform3(shaderLocationEnum location, int data1, int data2, int data3);
	void setUniform4(shaderLocationEnum location, int data1, int data2, int data3, int data4);
};

Shader * shader(std::string searchName);
Shader * addShader(std::string newName, std::string vertexShaderFile, std::string fragmentShaderFile, ...);
void destroyShader(std::string searchName);
void destroyAllShaders();

}

#endif /* SHADER_H_ */
