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

#include <iostream>
#include <vector>

typedef int GLint;
typedef unsigned GLuint;
typedef float GLfloat;

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
struct vec2;
struct vec3;
struct vec4;

class Shader {
	GLuint program_;
	GLint uniformLocation_[SHADER_LOCATION_ENUM_COUNT];
	std::string name_;

public:
	friend class Sprite;
	friend class Model;
	friend class Font;

	operator GLuint() const;

	Shader(const std::string & name, const std::string & vertexShaderFile, const std::string & fragmentShaderFile,
			...);
	Shader(const std::string & name, const std::string & vertexShaderFile, const std::string & fragmentShaderFile,
			const std::vector<int> & enums, const std::vector<char *> & attributeNames);
	Shader(const std::string & name, const std::string & vertexShaderFile, const std::string & fragmentShaderFile,
			unsigned count, int * enums, const char ** attributeNames);
	~Shader();

	const std::string & name() const;
	void bind();
	void use() const;
	GLint setUniformLocation(shaderLocationEnum dstLocation, const std::string & locationName);
	GLint uniformLocation(shaderLocationEnum location) const;

	void setUniform1(shaderLocationEnum location, GLfloat * data, unsigned count = 1) const;
	void setUniform2(shaderLocationEnum location, GLfloat * data, unsigned count = 1) const;
	void setUniform3(shaderLocationEnum location, GLfloat * data, unsigned count = 1) const;
	void setUniform4(shaderLocationEnum location, GLfloat * data, unsigned count = 1) const;

	void setUniform9(shaderLocationEnum location, GLfloat * data, unsigned count = 1) const;
	void setUniform16(shaderLocationEnum location, GLfloat * data, unsigned count = 1) const;

	void setUniform1(shaderLocationEnum location, int * data, unsigned count = 1) const;
	void setUniform2(shaderLocationEnum location, int * data, unsigned count = 1) const;
	void setUniform3(shaderLocationEnum location, int * data, unsigned count = 1) const;
	void setUniform4(shaderLocationEnum location, int * data, unsigned count = 1) const;

	void setUniform1(shaderLocationEnum location, GLfloat data) const;
	void setUniform2(shaderLocationEnum location, GLfloat data1, GLfloat data2) const;
	void setUniform3(shaderLocationEnum location, GLfloat data1, GLfloat data2, GLfloat data3) const;
	void setUniform4(shaderLocationEnum location, GLfloat data1, GLfloat data2, GLfloat data3, GLfloat data4) const;

	void setUniform1(shaderLocationEnum location, int data) const;
	void setUniform2(shaderLocationEnum location, int data1, int data2) const;
	void setUniform3(shaderLocationEnum location, int data1, int data2, int data3) const;
	void setUniform4(shaderLocationEnum location, int data1, int data2, int data3, int data4) const;

	void setUniform2(shaderLocationEnum location, const vec2 & data) const;
	void setUniform3(shaderLocationEnum location, const vec3 & data) const;
	void setUniform4(shaderLocationEnum location, const vec4 & data) const;
};

}

#endif /* SHADER_H_ */
