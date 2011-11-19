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
#include <GL/glew.h>
#include "../Display.h"

namespace SuperMaximo {

class Object;
class Shader;
class Model;

struct bone;
struct mat4;

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
	struct vertex {
		float x, y, z;
		vec3 normal_;
		vertex operator- (vertex const & other);
	};
	struct material {
		std::string name, fileName;
		bool hasTexture;
		vec3 ambientColor, diffuseColor, specularColor;
		float shininess, alpha;
	};
	struct triangle {
		vertex coords[3], texCoords[3];
		int mtlNum;

		vec3 surfaceNormal();
	};

	std::string name_;
	std::vector<triangle> triangles_;
	std::vector<material> materials_;
	std::vector<bone *> bones_;
	GLuint vao, vbo, texture;
	Shader * boundShader_;
	unsigned framerate_, vertexCount_, textureCount;

	void loadObj(const std::string & path, const std::string & fileName, bufferUsageEnum bufferUsage = STATIC_DRAW,
			void (*customBufferFunction)(GLuint*, Model*, void*) = NULL, void * customData = NULL);
	void loadSmm(const std::string & path, const std::string & fileName, bufferUsageEnum bufferUsage = STATIC_DRAW);
	void loadSms(const std::string & fileName);
	void loadSma(const std::string & fileName);
	void loadSmo(const std::string & path, const std::string & fileName, bufferUsageEnum bufferUsage = STATIC_DRAW);

	void initBufferObj(bufferUsageEnum bufferUsage);

	void getBoneModelviewMatrices(mat4 * matrixArray, bone * pBone);
	void setBoneRotationsFromAnimation(unsigned animationId, float frame, bone * pBone);

public:
	friend class Object;
	friend struct keyFrame;
	friend struct bone;
	Model(const std::string & newName, const std::string & path, const std::string & fileName, unsigned framerate = 60,
			bufferUsageEnum bufferUsage = STATIC_DRAW, void (*customBufferFunction)(GLuint*, Model*, void*) = NULL,
			void * customData = NULL);
	~Model();
	std::string name();
	void draw(Object * object, bool skipAnimation = false);
	void draw(float x, float y, float z, float xRotation = 0.0f, float yRotation = 0.0f, float zRotation = 0.0f,
			float xScale = 1.0f, float yScale = 1.0f, float zScale = 1.0f, float frame = 1.0f,
			int currentAnimationId = 0, bool skipAnimation = false);

	void bindShader(Shader * shader);
	Shader * boundShader();

	int boneId(const std::string & boneName);
	std::string boneName(unsigned boneId);

	int animationId(const std::string & searchName);
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
		int frameIndex(float step);
	};
	std::vector<animation> animations;
};


Model * model(const std::string & searchName);
Model * addModel(const std::string & newName, const std::string & path, const std::string & fileName,
		bufferUsageEnum bufferUsage = STATIC_DRAW, unsigned framerate = 60,
		void (*customBufferFunction)(GLuint*, Model*, void*) = NULL, void * customData = NULL);
void destroyModel(const std::string & searchName);
void destroyAllModels();

}

#endif /* MODEL_H_ */
