//============================================================================
// Name        : Model.h
// Author      : Max Foster
// Created on  : 30 May 2011
// Version     : 1.0
// Copyright   : http://creativecommons.org/licenses/by/3.0/
// Description : SuperMaximo GameLibrary 3D Model class
//============================================================================

#ifndef MODEL_H_
#define MODEL_H_

#include <iostream>
#include <vector>
#include <cmath>
#include <GL/gl.h>
#include "../Display.h"

namespace SuperMaximo {

class Object;
class Shader;
class Model;

struct bone;
struct matrix4d;

enum bufferUsageEnum {
	STREAM_DRAW = GL_STREAM_DRAW,
	STREAM_READ = GL_STREAM_READ,
	STREAM_COPY = GL_STREAM_COPY,
	STATIC_DRAW = GL_STATIC_DRAW,
	STATIC_READ = GL_STATIC_READ,
	STATIC_COPY = GL_STATIC_COPY,
	DYNAMIC_DRAW = GL_DYNAMIC_DRAW,
	DYNAMIC_READ = GL_DYNAMIC_READ,
	DYNAMIC_COPY = GL_DYNAMIC_COPY
};

class Model {
	/*struct normal {
		float x, y, z;
		normal operator- (normal const & other);
	};*/
	struct vertex {
		float x, y, z;
		vec3 normal_;
		//int boneId;
		vertex operator- (vertex const & other);
	};
	struct material {
		std::string name, fileName;
		//int textureId;
		bool hasTexture;
		vec3 ambientColor, diffuseColor, specularColor;
		float shininess, alpha;
	};
	struct triangle {
		vertex coords[3], texCoords[3];
		int mtlNum;//, boneId;
		//bone * pBone;
		//bool sharedCoord[3];
		vec3 surfaceNormal();
	};
	std::string name_;
	std::vector<triangle> triangles_;
	std::vector<material> materials_;
	std::vector<bone *> bones_;
	GLuint vao, vbo, texture;
	Shader * boundShader_;
	unsigned framerate_, vertexCount_;
	void loadObj(std::string path, std::string fileName, bufferUsageEnum bufferUsage = STATIC_DRAW,
			void (*customBufferFunction)(GLuint*, Model*, void*) = NULL, void * customData = NULL);
	void loadSmm(std::string path, std::string fileName, bufferUsageEnum bufferUsage = STATIC_DRAW);
	void loadSms(std::string fileName);
	void loadSma(std::string fileName);
	void loadSmo(std::string path, std::string fileName, bufferUsageEnum bufferUsage = STATIC_DRAW);
	//, void (*customBufferFunction)(GLuint*, Model*, void*) = NULL, void * customData = NULL);
	void initBufferObj(bufferUsageEnum bufferUsage);
	//void initBufferSmo();
	//void drawObj(Shader * shaderToUse);
	//vec3 calculatePoints(float nx, float ny, float nz, matrix4d matrix);
	//void calculateHitbox(bone * pBone, matrix4d matrix);
	//void drawBone(bone * pBone, Shader * shaderToUse, bool skipHitboxes);
	void getBoneModelviewMatrices(matrix4d * matrixArray, bone * pBone);
	void setBoneRotationsFromAnimation(unsigned animationId, float frame, bone * pBone);
public:
	friend class Object;
	friend struct keyFrame;
	friend struct bone;
	Model(std::string newName, std::string path, std::string fileName, unsigned framerate = 60,
			bufferUsageEnum bufferUsage = STATIC_DRAW, void (*customBufferFunction)(GLuint*, Model*, void*) = NULL,
			void * customData = NULL);
	~Model();
	std::string name();
	void draw(Object * object, bool skipAnimation = false);//, bool skipHitboxes = false);
	void draw(float x, float y, float z, float xRotation = 0.0f, float yRotation = 0.0f, float zRotation = 0.0f,
			float xScale = 1.0f, float yScale = 1.0f, float zScale = 1.0f, float frame = 1.0f,
			int currentAnimationId = 0, bool skipAnimation = false);
		//, bool skipHitboxes = false);

	void bindShader(Shader * shader);
	Shader * boundShader();

	int boneId(std::string boneName);
	std::string boneName(unsigned boneId);

	int animationId(std::string searchName);
	void setFramerate(unsigned newFramerate);
	unsigned framerate();

	std::vector<bone *> * bones();
	std::vector<triangle> * triangles();
	std::vector<material> * materials();

	GLuint * vboPointer();

	unsigned vertexCount();
};

struct bone {
	int id;
	//unsigned offset;
	std::string name;
	float x, y, z, endX, endY, endZ, xRot, yRot, zRot;
	bone * parent;
	vec3 rotationUpperLimit, rotationLowerLimit;
	std::vector<bone *> child;
	//vector<Model::triangle *> triangles;
	//vector<Model::vertex *> vertices;
	/*struct box {
		float x, y, z, l, w, h, xRot, yRot, zRot, rl, rw, rh;
		void init();
	} hitbox;*/
	struct keyFrame {
		/*struct keyFrameData {
			int boneId;
			float xRot, yRot, zRot;
		};
		vector<keyFrameData> boneData;*/
		float xRot, yRot, zRot;
		unsigned step;
		//void set(Model * model);
	};
	struct animation {
		std::string name;
		unsigned length;
		std::vector<keyFrame> frames;
		int frameIndex(unsigned step);
	};
	std::vector<animation> animations;
};


Model * model(std::string searchName);
Model * addModel(std::string newName, std::string path, std::string fileName,
		bufferUsageEnum bufferUsage = STATIC_DRAW, unsigned framerate = 60,
		void (*customBufferFunction)(GLuint*, Model*, void*) = NULL, void * customData = NULL);
void destroyModel(std::string searchName);
void destroyAllModels();

}

#endif /* MODEL_H_ */
