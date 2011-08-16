//============================================================================
// Name        : Object.h
// Author      : Max Foster
// Created on  : 6 May 2011
// Version     : 1.0
// Copyright   : http://creativecommons.org/licenses/by/3.0/
// Description : SuperMaximo GameLibrary game Object class
//============================================================================

#ifndef OBJECT_H_
#define OBJECT_H_

#include <iostream>
#include "../Display.h"

namespace SuperMaximo {

class Sprite;
class Model;
struct bone;
class Shader;

typedef void (*customDrawFunctionType)(void*, Shader*, void*);

class Object {
	Sprite * sprite_;
	Model * model_;
	bool hasModel_;//, interpolating;
	float x_, y_, z_, xRotation_, yRotation_, zRotation_, xScale_, yScale_, zScale_, width_, height_, alpha_,
		xRotatedWidth_, yRotatedWidth_, zRotatedWidth_, xRotatedHeight_, yRotatedHeight_, zRotatedHeight_, originX,
		originY;
	std::vector<unsigned> currentAnimationId;
	std::vector<float> frame_;
	std::string name_;
	Shader * boundShader_;
	customDrawFunctionType customDrawFunction;
	//keyFrame * fakeKeyFrame1, * fakeKeyFrame2;
public:
	friend class Sprite;
	friend class Model;
	Object(std::string newName, float destX, float destY, float destZ, Sprite * newSprite = NULL);
	Object(std::string newName, float destX, float destY, float destZ, Model * newModel = NULL);
	~Object();

	std::string name();
	void setSprite(Sprite * newSprite);
	Sprite * sprite();
	void setModel(Model * newModel);
	Model * model();
	bool hasModel();

	void bindShader(Shader * shader);
	Shader * boundShader();

	void bindCustomDrawFunction(customDrawFunctionType newCustomDrawFunction);
	customDrawFunctionType boundCustomDrawFunction();

	void setPosition(float xAmount, float yAmount, float zAmount, bool relative = false);
	void setPosition(vec2 amount, bool relative = false);
	void setPosition(vec3 amount, bool relative = false);
	float setX(float amount, bool relative = false);
	float setY(float amount, bool relative = false);
	float setZ(float amount, bool relative = false);
	float x();
	float y();
	float z();

	float width();
	float height();
	void calcZRotatedDimensions();

	void scale(float xAmount, float yAmount, float zAmount, bool relative = false,
			bool recalculateDimensions = true);
	float setXScale(float amount, bool relative = false);
	float setYScale(float amount, bool relative = false);
	float setZScale(float amount, bool relative = false);
	float xScale();
	float yScale();
	float zScale();

	void rotate(float xAmount, float yAmount, float zAmount, bool relative = false,
			bool recalculateDimensions = true);
	float rotate(float amount, bool relative = false, bool recalculateDimensions = true);
	float setXRotation(float amount, bool relative = false);
	float setYRotation(float amount, bool relative = false);
	float setZRotation(float amount, bool relative = false);
	float xRotation();
	float yRotation();
	float zRotation();

	float setAlpha(float amount, bool relative = false);
	float alpha();

	void setCurrentAnimation(unsigned animationId, int boneId = -1, bool withChildren = true);
	unsigned currentAnimation(int boneId = -1);

	void setFrame(float newFrame, bool relative = false, int boneId = -1, bool withChildren = true);
	float frame(int boneId = -1);

	//void animate(unsigned start, unsigned finish, unsigned animationId = 0);
	//void setInterpolation(int startFrame, unsigned animation1Id, int endFrame, unsigned animation2Id,
	//	unsigned numFramesToTake);
	//void setInterpolation();

	void draw(bool skipAnimation = false);//, bool skipHitboxes = false);

	bool roughMouseOverBox();
	bool mouseOverBox();
	bool mouseOverCircle();
	bool boxCollision(Object * other, bool allStages = true);
	bool roughBoxCollision(Object * other);
	bool circleCollision(Object * other);

	/*bool roughHitboxCollision(bone * bone1, bone * bone2);
	bool hitBoxCollision(bone * bone1, bone * bone2);
	bool roughModelCollision(Object * other, int hitboxId = -1, int hitboxOtherId = -1);
	bool modelCollision(Object * other, int hitboxId = -1, int hitboxOtherId = -1);*/
};

Object * object(std::string searchName);
Object * addObject(std::string newName, float destX, float destY, float destZ, Sprite * newSprite = NULL);
Object * addObject(std::string newName, float destX, float destY, float destZ, Model * newModel = NULL);
void destroyObject(std::string searchName);
void destroyAllObjects();

}

#endif /* OBJECT_H_ */
