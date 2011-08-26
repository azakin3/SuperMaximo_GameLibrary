//============================================================================
// Name        : Display.h
// Author      : Max Foster
// Created on  : 5 May 2011
// Version     : 1.0
// Copyright   : http://creativecommons.org/licenses/by/3.0/
// Description : SuperMaximo GameLibrary OpenGL functions and helpful types
//============================================================================

#ifndef DISPLAY_H_
#define DISPLAY_H_

#include <iostream>
#include <vector>
#include <GL/glew.h>

namespace SuperMaximo {

class Shader;

typedef void (*customDrawFunctionType)(void*, Shader*, void*);
typedef float float16[16];

enum textureUnitEnum {
	TEXTURE0 = GL_TEXTURE0,
	TEXTURE1 = GL_TEXTURE1,
	TEXTURE2 = GL_TEXTURE2,
	TEXTURE3 = GL_TEXTURE3,
	TEXTURE4 = GL_TEXTURE4,
	TEXTURE5 = GL_TEXTURE5,
	TEXTURE6 = GL_TEXTURE6,
	TEXTURE7 = GL_TEXTURE7,
	TEXTURE8 = GL_TEXTURE8,
	TEXTURE9 = GL_TEXTURE9,
	TEXTURE10 = GL_TEXTURE10,
	TEXTURE11 = GL_TEXTURE11,
	TEXTURE12 = GL_TEXTURE12,
	TEXTURE13 = GL_TEXTURE13,
	TEXTURE14 = GL_TEXTURE14,
	TEXTURE15 = GL_TEXTURE15
};

enum matrixEnum {
	MODELVIEW_MATRIX = 0,
	PERSPECTIVE_MATRIX,
	ORTHOGRAPHIC_MATRIX,
	PROJECTION_MATRIX,
	IDENTITY_MATRIX //Make sure IDENTITY_MATRIX is last
};

enum blendFuncEnum {
	ZERO = GL_ZERO,
	ONE = GL_ONE,
	SRC_COLOR = GL_SRC_COLOR,
	ONE_MINUS_SRC_COLOR = GL_ONE_MINUS_SRC_COLOR,
	DST_COLOR = GL_DST_COLOR,
	ONE_MINUS_DST_COLOR = GL_ONE_MINUS_DST_COLOR,
	SRC_ALPHA = GL_SRC_ALPHA,
	ONE_MINUS_SRC_ALPHA = GL_ONE_MINUS_SRC_ALPHA,
	DST_ALPHA = GL_DST_ALPHA,
	ONE_MINUS_DST_ALPHA = GL_ONE_MINUS_DST_ALPHA,
	CONSTANT_COLOR = GL_CONSTANT_COLOR,
	ONE_MINUS_CONSTANT_COLOR = GL_ONE_MINUS_CONSTANT_COLOR,
	CONSTANT_ALPHA = GL_CONSTANT_ALPHA,
	ONE_MINUS_CONSTANT_ALPHA = GL_ONE_MINUS_CONSTANT_ALPHA,
	SRC_ALPHA_SATURATE = GL_SRC_ALPHA_SATURATE
};

enum blendFuncEquEnum {
	FUNC_ADD = GL_FUNC_ADD,
	FUNC_SUBTRACT = GL_FUNC_SUBTRACT,
	FUNC_REVERSE_SUBTRACT = GL_FUNC_REVERSE_SUBTRACT,
	MIN = GL_MIN,
	MAX = GL_MAX
};

struct mat2 {
	float component[4];
	float & operator[](short i);
	mat2 operator*(const mat2 & otherMat);
	operator float*();
	void initIdentity();
};

struct mat3 {
	float component[9];
	float & operator[](short i);
	mat3 operator*(const mat3 & otherMat);
	operator float*();
	void initIdentity();
};

struct mat4 {
	float component[16];
	float & operator[](short i);
	mat4 operator*(const mat4 & otherMat);
	operator float*();
	void initIdentity();
};

struct vec2 {
	union {
		float x;
		float r;
		float s;
	};
	union {
		float y;
		float g;
		float t;
	};
	vec2 operator+(const vec2 & otherVector);
	vec2 operator-(const vec2 & otherVector);
	void operator+=(const vec2 & otherVector);
	void operator-=(const vec2 & otherVector);
	vec2 operator*(const mat2 & matrix);
	void operator*=(const mat2 & matrix);
	vec2 operator*(float num);
	void operator*=(float num);
	vec2 operator/(float num);
	void operator/=(float num);
	vec2 perpendicular();
	float dotProduct(const vec2 & otherVector);
	bool polygonCollision(unsigned vertexCount, ...);
	bool polygonCollision(unsigned vertexCount, vec2 * vertices);
	bool polygonCollision(unsigned vertexCount, std::vector<vec2> vertices);
};

struct vec3 {
	union {
		float x;
		float r;
		float s;
	};
	union {
		float y;
		float g;
		float t;
	};
	union {
		float z;
		float b;
		float p;
	};
	vec3 operator+(const vec3 & otherVector);
	vec3 operator-(const vec3 & otherVector);
	void operator+=(const vec3 & otherVector);
	void operator-=(const vec3 & otherVector);
	vec3 operator*(float num);
	void operator*=(float num);
	vec3 operator/(float num);
	void operator/=(float num);
	float dotProduct(const vec3 & otherVector);
};

struct vec4 {
	union {
		float x;
		float r;
		float s;
	};
	union {
		float y;
		float g;
		float t;
	};
	union {
		float z;
		float b;
		float p;
	};
	union {
		float w;
		float a;
		float q;
	};
	vec4 operator+(const vec4 & otherVector);
	vec4 operator-(const vec4 & otherVector);
	void operator+=(const vec4 & otherVector);
	void operator-=(const vec4 & otherVector);
	vec4 operator*(const mat4 & matrix);
	vec4 operator*(float num);
	void operator*=(float num);
	vec4 operator/(float num);
	void operator/=(float num);
	float dotProduct(const vec4 & otherVector);
	operator vec3();
};


bool initDisplay(unsigned width, unsigned height, unsigned depth, unsigned maxFramerate = 0,
		bool fullScreen = false, std::string windowTitle = "My Game");
void quitDisplay();

unsigned screenWidth();
unsigned screenHeight();
unsigned screenDepth();
bool resizeScreen(unsigned width, unsigned height, bool fullScreen = false);

void setClearColor(float r, float g, float b, float a);
void setClearColor(vec4 color);
vec4 getClearColor();

mat4 getPerspectiveMatrix(float left, float right, float bottom, float top, float front, float back);
mat4 getPerspectiveMatrix(float angle, float aspectRatio, float front, float back);
mat4 getOrthographicMatrix(float left, float right, float bottom, float top, float front, float back);

mat2 get2dRotationMatrix(float angle);

void bindShader(Shader * shader);
Shader * boundShader();

void bindCustomDrawFunction(customDrawFunctionType newCustomDrawFunction);
customDrawFunctionType boundCustomDrawFunction();

void bindTextureUnit(textureUnitEnum textureUnit);
textureUnitEnum boundTextureUnit();

void setMatrix(matrixEnum matrixId);
matrixEnum currentMatrix();
void copyMatrix(matrixEnum srcMatrixId, matrixEnum dstMatrixId);
void copyMatrix(mat4 srcMatrix, matrixEnum dstMatrixId);
mat4 getMatrix(matrixEnum matrixId);
void pushMatrix();
void popMatrix();

void translateMatrix(float x, float y, float z);
void rotateMatrix(float angle, float x, float y, float z);
void scaleMatrix(float xScale, float yScale, float zScale);

void refreshScreen();
unsigned getFramerate();
unsigned getTickDifference();
void setIdealFramerate(unsigned newIdealFramerate);
unsigned getIdealFramerate();
float compensation();

void enableBlending(blendFuncEnum srcBlendFunc = ONE, blendFuncEnum dstBlendFunc = ZERO,
		blendFuncEquEnum blendFuncEquation = FUNC_ADD);
void disableBlending();
bool blendingEnabled();

void enableDepthTesting();
void disableDepthTesting();
bool depthTestingEnabled();

float openGlVersion();
float glSlVersion();

bool vertexArrayObjectSupported();

void disableTexture2dArray();
bool texture2dArrayDisabled();

void disableTextureRectangle();
bool textureRectangleDisabled();

}

#endif /* DISPLAY_H_ */
