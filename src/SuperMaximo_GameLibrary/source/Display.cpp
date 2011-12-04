//============================================================================
// Name        : Display.cpp
// Author      : Max Foster
// Created on  : 5 May 2011
// Version     : 1.0
// Copyright   : http://creativecommons.org/licenses/by/3.0/
// Description : SuperMaximo GameLibrary OpenGL functions and helpful types
//============================================================================

#include <iostream>
#include <vector>
#include <cmath>
using namespace std;
#include <GL/glew.h>
#include <SDL/SDL_framerate.h>
#include <SDL/SDL_video.h>
#include <SDL/SDL_image.h>
#include "../headers/Display.h"
#include "../headers/classes/Shader.h"
#include "../headers/Input.h"
#include "../headers/Utils.h"
using namespace SuperMaximo;

SDL_Surface * screen;
unsigned screenW, screenH, screenD, framerate = 0, maximumFramerate, tickDifference = 1, idealFramerate = 60;
matrixEnum currentMatrixId;
mat4 matrix[IDENTITY_MATRIX+1];
vector<mat4> matrixStack[IDENTITY_MATRIX]; //We don't want a stack for the identity matrix.
											//Make sure IDENTITY_MATRIX enum is last
bool blendingEnabled_ = false, depthTestingEnabled_ = true, texture2dArrayDisabled_ = false,
		textureRectangleDisabled_ = false;
Shader * boundShader_ = NULL;
customDrawFunctionType customDrawFunction = NULL;
textureUnitEnum boundTexureUnit_ = TEXTURE0;
Uint32 ticks = 0, lastTicks = 0;
float compensation_ = 1.0f;
vec4 clearColor;

namespace SuperMaximo {

float & mat2::operator[](short i) {
	return component[i];
}

mat2 mat2::operator*(const mat2 & otherMat) {
	mat2 returnMatrix;
	returnMatrix.component[0] = (component[0]*otherMat.component[0])+(component[2]*otherMat.component[1]);
	returnMatrix.component[1] = (component[1]*otherMat.component[0])+(component[3]*otherMat.component[1]);

	returnMatrix.component[2] = (component[0]*otherMat.component[2])+(component[2]*otherMat.component[3]);
	returnMatrix.component[3] = (component[1]*otherMat.component[2])+(component[3]*otherMat.component[3]);

	return returnMatrix;
}

mat2::operator float*() {
	return component;
}

void mat2::initIdentity() {
	component[0] = component[3] = 1.0f;
	component[1] = component[2] = 0.0f;
}

float & mat3::operator[](short i) {
	return component[i];
}

mat3 mat3::operator*(const mat3 & otherMat) {
	mat3 returnMatrix;
	returnMatrix.component[0] = (component[0]*otherMat.component[0])+(component[3]*otherMat.component[1])+
			(component[6]*otherMat.component[2]);
	returnMatrix.component[1] = (component[1]*otherMat.component[0])+(component[4]*otherMat.component[1])+
			(component[7]*otherMat.component[2]);
	returnMatrix.component[2] = (component[2]*otherMat.component[0])+(component[5]*otherMat.component[1])+
			(component[8]*otherMat.component[2]);

	returnMatrix.component[3] = (component[0]*otherMat.component[3])+(component[3]*otherMat.component[4])+
			(component[6]*otherMat.component[5]);
	returnMatrix.component[4] = (component[1]*otherMat.component[3])+(component[4]*otherMat.component[4])+
			(component[7]*otherMat.component[5]);
	returnMatrix.component[5] = (component[2]*otherMat.component[3])+(component[5]*otherMat.component[4])+
			(component[8]*otherMat.component[5]);

	returnMatrix.component[6] = (component[0]*otherMat.component[6])+(component[3]*otherMat.component[7])+
			(component[6]*otherMat.component[8]);
	returnMatrix.component[7] = (component[1]*otherMat.component[6])+(component[4]*otherMat.component[7])+
			(component[7]*otherMat.component[8]);
	returnMatrix.component[8] = (component[2]*otherMat.component[6])+(component[5]*otherMat.component[7])+
			(component[8]*otherMat.component[8]);

	return returnMatrix;
}

mat3::operator float*() {
	return component;
}

void mat3::initIdentity() {
	for (short i = 0; i < 9; i++) component[i] = 0.0f;
	component[0] = 1.0f;
	component[4] = 1.0f;
	component[8] = 1.0f;
}

float & mat4::operator[](short i) {
	return component[i];
}

mat4 mat4::operator*(const mat4 & otherMat) {
	mat4 returnMatrix;
	returnMatrix.component[0] = (component[0]*otherMat.component[0])+(component[4]*otherMat.component[1])+
			(component[8]*otherMat.component[2])+(component[12]*otherMat.component[3]);
	returnMatrix.component[1] = (component[1]*otherMat.component[0])+(component[5]*otherMat.component[1])+
			(component[9]*otherMat.component[2])+(component[13]*otherMat.component[3]);
	returnMatrix.component[2] = (component[2]*otherMat.component[0])+(component[6]*otherMat.component[1])+
			(component[10]*otherMat.component[2])+(component[14]*otherMat.component[3]);
	returnMatrix.component[3] = (component[3]*otherMat.component[0])+(component[7]*otherMat.component[1])+
			(component[11]*otherMat.component[2])+(component[15]*otherMat.component[3]);

	returnMatrix.component[4] = (component[0]*otherMat.component[4])+(component[4]*otherMat.component[5])+
			(component[8]*otherMat.component[6])+(component[12]*otherMat.component[7]);
	returnMatrix.component[5] = (component[1]*otherMat.component[4])+(component[5]*otherMat.component[5])+
			(component[9]*otherMat.component[6])+(component[13]*otherMat.component[7]);
	returnMatrix.component[6] = (component[2]*otherMat.component[4])+(component[6]*otherMat.component[5])+
			(component[10]*otherMat.component[6])+(component[14]*otherMat.component[7]);
	returnMatrix.component[7] = (component[3]*otherMat.component[4])+(component[7]*otherMat.component[5])+
			(component[11]*otherMat.component[6])+(component[15]*otherMat.component[7]);

	returnMatrix.component[8] = (component[0]*otherMat.component[8])+(component[4]*otherMat.component[9])+
			(component[8]*otherMat.component[10])+(component[12]*otherMat.component[11]);
	returnMatrix.component[9] = (component[1]*otherMat.component[8])+(component[5]*otherMat.component[9])+
			(component[9]*otherMat.component[10])+(component[13]*otherMat.component[11]);
	returnMatrix.component[10] = (component[2]*otherMat.component[8])+(component[6]*otherMat.component[9])+
			(component[10]*otherMat.component[10])+(component[14]*otherMat.component[11]);
	returnMatrix.component[11] = (component[3]*otherMat.component[8])+(component[7]*otherMat.component[9])+
			(component[11]*otherMat.component[10])+(component[15]*otherMat.component[11]);

	returnMatrix.component[12] = (component[0]*otherMat.component[12])+(component[4]*otherMat.component[13])+
			(component[8]*otherMat.component[14])+(component[12]*otherMat.component[15]);
	returnMatrix.component[13] = (component[1]*otherMat.component[12])+(component[5]*otherMat.component[13])+
			(component[9]*otherMat.component[14])+(component[13]*otherMat.component[15]);
	returnMatrix.component[14] = (component[2]*otherMat.component[12])+(component[6]*otherMat.component[13])+
			(component[10]*otherMat.component[14])+(component[14]*otherMat.component[15]);
	returnMatrix.component[15] = (component[3]*otherMat.component[12])+(component[7]*otherMat.component[13])+
			(component[11]*otherMat.component[14])+(component[15]*otherMat.component[15]);

	return returnMatrix;
}

mat4::operator float*() {
	return component;
}

void mat4::initIdentity() {
	for (short i = 0; i < 16; i++) component[i] = 0.0f;
	component[0] = 1.0f;
	component[5] = 1.0f;
	component[10] = 1.0f;
	component[15] = 1.0f;
}

vec2::vec2(float x, float y) : x(x), y(y) {}

vec2 vec2::operator+(const vec2 & otherVector) {
	return vec2(x+otherVector.x, y+otherVector.y);
}

vec2 vec2::operator-(const vec2 & otherVector) {
	return vec2(x-otherVector.x, y-otherVector.y);
}

void vec2::operator+=(const vec2 & otherVector) {
	x += otherVector.x;
	y += otherVector.y;
}

void vec2::operator-=(const vec2 & otherVector) {
	x -= otherVector.x;
	y -= otherVector.y;
}

vec2 vec2::operator*(const mat2 & matrix) {
	return vec2(
		(x*matrix.component[0])+(y*matrix.component[2]),
		(x*matrix.component[1])+(y*matrix.component[3]));
}

void vec2::operator*=(const mat2 & matrix) {
	*this = (*this)*matrix;
}

vec2 vec2::operator*(float num) {
	return vec2(x*num, y*num);
}

void vec2::operator*=(float num) {
	x *= num, y *= num;
}

vec2 vec2::operator/(float num) {
	return vec2(x/num, y/num);
}

void vec2::operator/=(float num) {
	x /= num, y /= num;
}

vec2 vec2::perpendicular() {
	return vec2(-y, x);
}

float vec2::dotProduct(const vec2 & otherVector) {
	return (x*otherVector.x)+(y*otherVector.y);
}

bool vec2::polygonCollision(unsigned vertexCount, ...) {
	const unsigned maxVertices = 100;
	if (vertexCount > maxVertices) vertexCount = maxVertices;

	va_list vertexArgs;
	va_start(vertexArgs, vertexCount);
	vec2 vertices[maxVertices];
	for (unsigned i = 0; i < vertexCount; i++) {
		vertices[i] = va_arg(vertexArgs, vec2);
	}
	va_end(vertexArgs);

	for (unsigned i = 1; i < vertexCount; i++) {
		if ((vertices[i-1]-(*this)).dotProduct((vertices[i]-vertices[i-1]).perpendicular()) < 0.0f) return false;
	}
	if ((vertices[vertexCount-1]-(*this)).dotProduct((vertices[0]-vertices[vertexCount-1]).perpendicular()) < 0.0f)
		return false;

	return true;
}

bool vec2::polygonCollision(unsigned vertexCount, vec2 * vertices) {
	for (unsigned i = 1; i < vertexCount; i++) {
		if ((vertices[i-1]-(*this)).dotProduct((vertices[i]-vertices[i-1]).perpendicular()) < 0.0f) return false;
	}
	if ((vertices[vertexCount-1]-(*this)).dotProduct((vertices[0]-vertices[vertexCount-1]).perpendicular()) < 0.0f)
		return false;

	return true;
}

bool vec2::polygonCollision(unsigned vertexCount, std::vector<vec2> vertices) {
	for (unsigned i = 1; i < vertexCount; i++) {
		if ((vertices[i-1]-(*this)).dotProduct((vertices[i]-vertices[i-1]).perpendicular()) < 0.0f) return false;
	}
	if ((vertices[vertexCount-1]-(*this)).dotProduct((vertices[0]-vertices[vertexCount-1]).perpendicular()) < 0.0f)
		return false;

	return true;
}

vec2::operator SuperMaximo::vec3() {
	return vec3(x, y, 0.0f);
}

vec2::operator SuperMaximo::vec4() {
	return vec4(x, y, 0.0f, 0.0f);
}

vec3::vec3(float x, float y, float z) : x(x), y(y), z(z) {}

vec3 vec3::operator+(const vec3 & otherVector) {
	return vec3(x+otherVector.x, y+otherVector.y, z+otherVector.z);
}

vec3 vec3::operator-(const vec3 & otherVector) {
	return vec3(x-otherVector.x, y-otherVector.y, z-otherVector.z);
}

void vec3::operator+=(const vec3 & otherVector) {
	x += otherVector.x;
	y += otherVector.y;
	z += otherVector.z;
}

void vec3::operator-=(const vec3 & otherVector) {
	x -= otherVector.x;
	y -= otherVector.y;
	z -= otherVector.z;
}

vec3 vec3::operator*(float num) {
	return vec3(x*num, y*num, z*num);
}

void vec3::operator*=(float num) {
	x *= num, y *= num, z *= num;
}

vec3 vec3::operator/(float num) {
	return vec3(x/num, y/num, z/num);
}

void vec3::operator/=(float num) {
	x /= num, y /= num, z /= num;
}

float vec3::dotProduct(const vec3 & otherVector) {
	return (x*otherVector.x)+(y*otherVector.y)+(z*otherVector.z);
}

vec3::operator SuperMaximo::vec2() {
	return vec2(x, y);
}

vec3::operator SuperMaximo::vec4() {
	return vec4(x, y, z, 0.0f);
}

vec4::vec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}

vec4 vec4::operator+(const vec4 & otherVector) {
	return vec4(x+otherVector.x, y+otherVector.y, z+otherVector.z, w+otherVector.w);
}

vec4 vec4::operator-(const vec4 & otherVector) {
	return vec4(x-otherVector.x, y-otherVector.y, z-otherVector.z, w-otherVector.w);
}

void vec4::operator+=(const vec4 & otherVector) {
	x += otherVector.x;
	y += otherVector.y;
	z += otherVector.z;
	w += otherVector.w;
}

void vec4::operator-=(const vec4 & otherVector) {
	x -= otherVector.x;
	y -= otherVector.y;
	z -= otherVector.z;
	w -= otherVector.w;
}

vec4 vec4::operator*(const mat4 & matrix) {
	vec4 returnVector;

	returnVector.x = (x*matrix.component[0])+(y*matrix.component[4])+(z*matrix.component[8])+(w*matrix.component[12]);
	returnVector.y = (x*matrix.component[1])+(y*matrix.component[5])+(z*matrix.component[9])+(w*matrix.component[13]);
	returnVector.z = (x*matrix.component[2])+(y*matrix.component[6])+(z*matrix.component[10])+(w*matrix.component[14]);
	returnVector.w = (x*matrix.component[3])+(y*matrix.component[7])+(z*matrix.component[11])+(w*matrix.component[15]);

	return returnVector;
}

vec4 vec4::operator*(float num) {
	return vec4(x*num, y*num, z*num, w*num);
}

void vec4::operator*=(float num) {
	x *= num, y *= num, z *= num, w /= num;
}

vec4 vec4::operator/(float num) {
	return vec4(x/num, y/num, z/num, w/num);
}

void vec4::operator/=(float num) {
	x /= num, y /= num, z /= num, w /= num;
}

float vec4::dotProduct(const vec4 & otherVector) {
	return (x*otherVector.x)+(y*otherVector.y)+(z*otherVector.z)+(w*otherVector.w);
}

vec4::operator SuperMaximo::vec2() {
	return vec2(x, y);
}

vec4::operator SuperMaximo::vec3() {
	return vec3(x, y, z);
}

bool initDisplay(unsigned width, unsigned height, unsigned depth, unsigned maxFramerate, bool fullScreen,
		const string & windowTitle) {
	if ((width > 0) && (height > 0)) {
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		if (fullScreen) screen = SDL_SetVideoMode(width, height, 32, SDL_OPENGL | SDL_FULLSCREEN);
			else screen = SDL_SetVideoMode(width, height, 32, SDL_OPENGL);
		if (screen == NULL) {
			return false;
		}
		SDL_WM_SetCaption(windowTitle.c_str(), windowTitle.c_str());
		screenW = width, screenH = height, screenD = depth;
		maximumFramerate = maxFramerate;
		if (maxFramerate > 0) idealFramerate = maxFramerate;
		ticks = SDL_GetTicks();

		if (glewInit() != GLEW_OK) return false;

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glDisable(GL_BLEND);

		matrix[IDENTITY_MATRIX].initIdentity();
		matrix[MODELVIEW_MATRIX].initIdentity();
		matrix[PERSPECTIVE_MATRIX] = getPerspectiveMatrix(45.0f, (float)width/(float)height, 1.0f, depth);
		matrix[ORTHOGRAPHIC_MATRIX] = getOrthographicMatrix(0.0f, screenW, screenH, 0.0f, 1.0f, depth);

		matrixStack[MODELVIEW_MATRIX].push_back(matrix[MODELVIEW_MATRIX]);
		matrixStack[PERSPECTIVE_MATRIX].push_back(matrix[PERSPECTIVE_MATRIX]);
		matrixStack[ORTHOGRAPHIC_MATRIX].push_back(matrix[ORTHOGRAPHIC_MATRIX]);
		matrixStack[PROJECTION_MATRIX].push_back(matrix[PROJECTION_MATRIX]);
		currentMatrixId = MODELVIEW_MATRIX;

		glViewport(0, 0, width, height);

		glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		return true;
	}
	return false;
}

void quitDisplay() {

}

unsigned screenWidth() {
	return screenW;
}

unsigned screenHeight() {
	return screenH;
}

unsigned screenDepth() {
	return screenD;
}

bool resizeScreen(unsigned width, unsigned height, bool fullScreen) {
	if ((width > 0) && (height > 0) && (matrixStack[ORTHOGRAPHIC_MATRIX].size() == 1)) {
		SDL_FreeSurface(screen);
		if (fullScreen) screen = SDL_SetVideoMode(width, height, 32, SDL_OPENGL | SDL_FULLSCREEN);
			else screen = SDL_SetVideoMode(width, height, 32, SDL_OPENGL);
		if (screen == NULL) {
			return false;
		}
		screenW = width, screenH = height;
		matrix[ORTHOGRAPHIC_MATRIX] = getOrthographicMatrix(0.0f, screenW, screenH, 0.0f, 1.0f, screenD);
		matrix[PERSPECTIVE_MATRIX] = getPerspectiveMatrix(45.0f, (float)screenW/(float)screenH, 1.0f, screenD);
		glViewport(0, 0, width, height);
		return true;
	}
	return false;
}

void setClearColor(float r, float g, float b, float a) {
	clearColor = vec4(r, g, b, a);
	glClearColor(r, g, b, a);
}

void setClearColor(vec4 color) {
	clearColor = color;
	glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
}

vec4 getClearColor() {
	return clearColor;
}

mat4 getPerspectiveMatrix(float left, float right, float bottom, float top, float front, float back) {
	mat4 returnMatrix;
	returnMatrix.component[0] = (2.0f*front)/(right-left);
	returnMatrix.component[1] = 0.0f;
	returnMatrix.component[2] = 0.0f;
	returnMatrix.component[3] = 0.0f;

	returnMatrix.component[4] = 0.0f;
	returnMatrix.component[5] = (2.0f*front)/(top-bottom);
	returnMatrix.component[6] = 0.0f;
	returnMatrix.component[7] = 0.0f;

	returnMatrix.component[8] = (right+left)/(right-left);
	returnMatrix.component[9] = (top+bottom)/(top-bottom);
	returnMatrix.component[10] = (-(back+front))/(back-front);
	returnMatrix.component[11] = -1.0f;

	returnMatrix.component[12] = 0.0f;
	returnMatrix.component[13] = 0.0f;
	returnMatrix.component[14] = (-2.0f*back*front)/(back-front);
	returnMatrix.component[15] = 0.0f;
	return returnMatrix;
}

mat4 getPerspectiveMatrix(float angle, float aspectRatio, float front, float back) {
	float tangent = tan((angle/2.0f)*(M_PI/180.0f));
	float height = front*tangent;
	float width = height*aspectRatio;

	mat4 returnMatrix = getPerspectiveMatrix(-width, width, -height, height, front, back);
	return returnMatrix;
}

mat4 getOrthographicMatrix(float left, float right, float bottom, float top, float front, float back) {
	mat4 returnMatrix;
	returnMatrix.component[0] = 2.0f/(right-left);
	returnMatrix.component[1] = 0.0f;
	returnMatrix.component[2] = 0.0f;
	returnMatrix.component[3] = 0.0f;

	returnMatrix.component[4] = 0.0f;
	returnMatrix.component[5] = 2.0f/(top-bottom);
	returnMatrix.component[6] = 0.0f;
	returnMatrix.component[7] = 0.0f;

	returnMatrix.component[8] = 0.0f;
	returnMatrix.component[9] = 0.0f;
	returnMatrix.component[10] = -2.0f/(back-front);
	returnMatrix.component[11] = 0.0f;

	returnMatrix.component[12] = -((right+left)/(right-left));
	returnMatrix.component[13] = -((top+bottom)/(top-bottom));
	returnMatrix.component[14] = -((back+front)/(back-front));
	returnMatrix.component[15] = 1.0f;
	return returnMatrix;
}

mat2 get2dRotationMatrix(float angle) {
	angle = (angle*M_PI)/180.0f;

	mat2 returnMatrix;

	returnMatrix.component[0] = cos(angle);
	returnMatrix.component[1] = sin(angle);
	returnMatrix.component[2] = -sin(angle);
	returnMatrix.component[3] = cos(angle);

	return returnMatrix;
}

void bindShader(Shader * shader) {
	shader->use();
	boundShader_ = shader;
}

Shader * boundShader() {
	return boundShader_;
}

void bindCustomDrawFunction(customDrawFunctionType newCustomDrawFunction) {
	customDrawFunction = newCustomDrawFunction;
}

customDrawFunctionType boundCustomDrawFunction() {
	return customDrawFunction;
}

void bindTextureUnit(textureUnitEnum textureUnit) {
	glActiveTexture(textureUnit);
	boundTexureUnit_ = textureUnit;
}

textureUnitEnum boundTextureUnit() {
	return boundTexureUnit_;
}

void setMatrix(matrixEnum matrixId) {
	if (currentMatrixId != IDENTITY_MATRIX) currentMatrixId = matrixId;
}

matrixEnum currentMatrix() {
	return currentMatrixId;
}

void copyMatrix(matrixEnum srcMatrixId, matrixEnum dstMatrixId) {
	if (dstMatrixId == IDENTITY_MATRIX) return;
	matrix[dstMatrixId] = matrix[srcMatrixId];
}

void copyMatrix(mat4 srcMatrix, matrixEnum dstMatrixId) {
	if (dstMatrixId == IDENTITY_MATRIX) return;
	matrix[dstMatrixId] = srcMatrix;
}

mat4 getMatrix(matrixEnum matrixId) {
	return matrix[matrixId];
}

void pushMatrix() {
	matrixStack[currentMatrixId].push_back(matrix[currentMatrixId]);
}

void popMatrix() {
	matrix[currentMatrixId] = matrixStack[currentMatrixId].back();
	if (matrixStack[currentMatrixId].size() > 1) matrixStack[currentMatrixId].pop_back();
}

void translateMatrix(float x, float y, float z) {
	mat4 transformationMatrix;
	transformationMatrix.initIdentity();
	transformationMatrix.component[12] = x;
	transformationMatrix.component[13] = y;
	transformationMatrix.component[14] = z;
	matrix[currentMatrixId] = matrix[currentMatrixId]*transformationMatrix;
}

void rotateMatrix(float angle, float x, float y, float z) {
	angle = (angle*M_PI)/180.0f;
	float len = sqrt((x*x)+(y*y)+(z*z));
	x /= len;
	y /= len;
	z /= len;
	mat4 transformationMatrix;

	float c = cos(angle), s = sin(angle), x2 = x*x, y2 = y*y, z2 = z*z;
	float t = 1.0f-c;

	transformationMatrix.component[0] = (x2*t)+c;
	transformationMatrix.component[1] = (y*x*t)+z*s;
	transformationMatrix.component[2] = (x*z*t)-(y*s);
	transformationMatrix.component[3] = 0.0f;

	transformationMatrix.component[4] = (x*y*t)-(z*s);
	transformationMatrix.component[5] = (y2*t)+c;
	transformationMatrix.component[6] = (y*z*t)+(x*s);
	transformationMatrix.component[7] = 0.0f;

	transformationMatrix.component[8] = (x*z*t)+(y*s);
	transformationMatrix.component[9] = (y*z*t)-(x*s);
	transformationMatrix.component[10] = (z2*t)+c;
	transformationMatrix.component[11] = 0.0f;

	transformationMatrix.component[12] = 0.0f;
	transformationMatrix.component[13] = 0.0f;
	transformationMatrix.component[14] = 0.0f;
	transformationMatrix.component[15] = 1.0f;

	matrix[currentMatrixId] = matrix[currentMatrixId]*transformationMatrix;
}

void scaleMatrix(float xScale, float yScale, float zScale) {
	mat4 transformationMatrix;
	transformationMatrix.initIdentity();
	transformationMatrix.component[0] = xScale;
	transformationMatrix.component[5] = yScale;
	transformationMatrix.component[10] = zScale;
	matrix[currentMatrixId] = matrix[currentMatrixId]*transformationMatrix;
}

void refreshScreen() {
	SDL_GL_SwapBuffers();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	resetEvents();

	lastTicks = ticks;
	ticks = SDL_GetTicks();
	tickDifference = ticks-lastTicks;
	if (tickDifference == 0) tickDifference = 1;
	framerate = 1000.0f/tickDifference;
	int delay = 0;
	if (maximumFramerate > 0) delay = (1000/maximumFramerate)-tickDifference;
	if (delay >= 0) SDL_Delay(delay);
	compensation_ = float(getTickDifference())/(1000.0f/float(getIdealFramerate()));
}

unsigned getFramerate() {
	return framerate;
}

unsigned getTickDifference() {
	return tickDifference;
}

void setIdealFramerate(unsigned newIdealFramerate) {
	idealFramerate = newIdealFramerate;
	if (idealFramerate == 0) idealFramerate = 60;
}

unsigned getIdealFramerate() {
	return idealFramerate;
}

float compensation() {
	return compensation_;
}

void enableBlending(blendFuncEnum srcBlendFunc, blendFuncEnum dstBlendFunc, blendFuncEquEnum blendFuncEquation) {
	if (!blendingEnabled_) {
		glEnable(GL_BLEND);
		blendingEnabled_ = true;
	}
	glBlendEquation(blendFuncEquation);
	glBlendFunc(srcBlendFunc, dstBlendFunc);
}

void disableBlending() {
	if (blendingEnabled_) {
		glDisable(GL_BLEND);
		blendingEnabled_ = false;
	}
}

bool blendingEnabled() {
	return blendingEnabled_;
}

void enableDepthTesting() {
	if (!depthTestingEnabled_) {
		glEnable(GL_DEPTH_TEST);
		depthTestingEnabled_ = true;
	}
}

void disableDepthTesting() {
	if (depthTestingEnabled_) {
		glDisable(GL_DEPTH_TEST);
		depthTestingEnabled_ = false;
	}
}

bool depthTestingEnabled() {
	return depthTestingEnabled_;
}

float openGlVersion() {
	static float version = 0.0f;
	if (version == 0.0f) {
		string str = reinterpret_cast<char const *>(glGetString(GL_VERSION));
		version = strtof(leftStr(str, 3).c_str(), NULL);
	}
	return version;
}

float glSlVersion() {
	static float version = 0.0f;
	if (version == 0.0f) {
		string str = reinterpret_cast<char const *>(glGetString(GL_SHADING_LANGUAGE_VERSION));
		version = strtof(leftStr(str, 3).c_str(), NULL);
	}
	return version;
}

bool vertexArrayObjectSupported() {
	static bool supported = (openGlVersion() >= 3.0f);
	static bool checked = false;
	if (!checked && !supported) {
		string str = reinterpret_cast<char const *>(glGetString(GL_EXTENSIONS));
		supported = ((str.find("GL_ARB_vertex_array_object") != string::npos)
				|| (str.find("GL_ATI_vertex_array_object") != string::npos)
				|| (str.find("GL_APPLE_vertex_array_object") != string::npos));
		checked = true;
	}
	return supported;
}

void disableTexture2dArray() {
	texture2dArrayDisabled_ = true;
}

bool texture2dArrayDisabled() {
	return texture2dArrayDisabled_;
}

void disableTextureRectangle() {
	textureRectangleDisabled_ = true;
}

bool textureRectangleDisabled() {
	return textureRectangleDisabled_;
}

}
