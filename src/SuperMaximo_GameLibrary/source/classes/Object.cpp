//============================================================================
// Name        : Object.cpp
// Author      : Max Foster
// Created on  : 6 May 2011
// Version     : 1.0
// Copyright   : http://creativecommons.org/licenses/by/3.0/
// Description : SuperMaximo GameLibrary game Object class
//============================================================================

#include <iostream>
#include <vector>
#include <cmath>
using namespace std;
#include <GL/glew.h>
#include <SDL/SDL_video.h>
#include "../../headers/classes/Object.h"
#include "../../headers/classes/Sprite.h"
#include "../../headers/classes/Model.h"
#include "../../headers/Display.h"
#include "../../headers/Utils.h"
#include "../../headers/Input.h"
using namespace SuperMaximo;

vector<Object*> allObjects[27];

namespace SuperMaximo {

Object::Object(const string & newName, float destX, float destY, float destZ, Sprite * newSprite) {
	name_ = newName, x_ = destX, y_ = destY, z_ = destZ, sprite_ = newSprite;
	frame_.push_back(0);
	xRotation_ = 0.0f, yRotation_ = 0.0f, zRotation_ = 0.0f, xScale_ = 1.0f, yScale_ = 1.0f, zScale_ = 1.0f;
	alpha_ = 1.0f, hasModel_ = false;

	if (sprite_ != NULL) {
		width_ = sprite_->rect.w, height_ = sprite_->rect.h, originX = sprite_->originX_, originY = sprite_->originY_;
	}
	zRotatedWidth_ = width_, zRotatedHeight_ = height_;
	boundShader_ = NULL;
	customDrawFunction = NULL;
}

Object::Object(const string & newName, float destX, float destY, float destZ, Model * newModel) {
	name_ = newName, x_ = destX, y_ = destY, z_ = destZ, model_ = newModel;

	if (model_ != NULL) {
		for (unsigned i = 0; i < model_->bones_.size(); i++) {
			frame_.push_back(0);
			currentAnimationId.push_back(0);
		}
	}
	xRotation_ = 0.0f, yRotation_ = 0.0f, zRotation_ = 0.0f, xScale_ = 1.0f, yScale_ = 1.0f, zScale_= 1.0f;
	alpha_ = 1.0f;
	hasModel_ = true;
	boundShader_ = NULL;
	customDrawFunction = NULL;
}

string Object::name() {
	return name_;
}

void Object::setSprite(Sprite * newSprite) {
	sprite_ = newSprite;
	if (sprite_ != NULL)
		width_ = sprite_->rect.w, height_ = sprite_->rect.h, originX = sprite_->originX_, originY = sprite_->originY_;
	hasModel_ = false;

	frame_.clear();
	currentAnimationId.clear();
	frame_.push_back(0);
}

Sprite * Object::sprite() {
	return sprite_;
}

void Object::setModel(Model * newModel) {
	model_ = newModel;
	hasModel_ = true;

	frame_.clear();
	currentAnimationId.clear();
	if (model_ != NULL) {
		for (unsigned i = 0; i < model_->bones_.size(); i++) {
			frame_.push_back(0);
			currentAnimationId.push_back(0);
		}
	}
}

Model * Object::model() {
	return model_;
}

bool Object::hasModel() {
	return hasModel_;
}

void Object::bindShader(Shader * shader) {
	boundShader_ = shader;
}

Shader * Object::boundShader() {
	return boundShader_;
}

void Object::bindCustomDrawFunction(customDrawFunctionType newCustomDrawFunction) {
	customDrawFunction = newCustomDrawFunction;
}

customDrawFunctionType Object::boundCustomDrawFunction() {
	return customDrawFunction;
}

void Object::setPosition(float xAmount, float yAmount, float zAmount, bool relative) {
	if (relative) x_ += xAmount*compensation(), y_ += yAmount*compensation(), z_ += zAmount*compensation();
		else x_ = xAmount, y_ = yAmount, z_ = zAmount;
}

void Object::setPosition(vec2 amount, bool relative) {
	if (relative) x_ += amount.x*compensation(), y_ += amount.y*compensation(); else x_ = amount.x, y_ = amount.y;
}

void Object::setPosition(vec3 amount, bool relative) {
	if (relative) x_ += amount.x*compensation(), y_ += amount.y*compensation(), z_ += amount.z*compensation();
		else x_ = amount.x, y_ = amount.y, z_ = amount.z;
}

float Object::setX(float amount, bool relative) {
	if (relative) x_ += amount*compensation(); else x_ = amount;
	return x_;
}

float Object::setY(float amount, bool relative) {
	if (relative) y_ += amount*compensation(); else y_ = amount;
	return y_;
}

float Object::setZ(float amount, bool relative) {
	if (relative) z_ += amount*compensation(); else z_ = amount;
	return z_;
}

float Object::x() {
	return x_;
}

float Object::y() {
	return y_;
}

float Object::z() {
	return z_;
}

float Object::width() {
	return width_;
}

float Object::height() {
	return height_;
}

void Object::calcZRotatedDimensions() {
	if ((zRotation_ == 0.0f) || (zRotation_ == 180.0f)) {
		zRotatedWidth_ = width_;
		zRotatedHeight_ = height_;
	} else if ((zRotation_ == 90.0f) || (zRotation_ == 270.0f)) {
		zRotatedWidth_ = height_;
		zRotatedHeight_ = width_;
	} else {
		float pointX[4], pointY[4];
		pointX[0] = -originX;
		pointY[0] = -originY;

		pointX[1] = -originX;
		pointY[1] = -originY+height_;

		pointX[2] = -originX+width_;
		pointY[2] = -originY+height_;

		pointX[3] = -originX+width_;
		pointY[3] = -originY;

		float zRotToRad = (zRotation_*M_PI)/180.0f;
		for (short i = 0; i < 4; i++) {
			float pointDistSquared = (pointX[i]*pointX[i])+(pointY[i]*pointY[i]);
			float angle = atan2(pointY[i], pointX[i]);
			float calc = angle-zRotToRad;
			pointX[i] = pointDistSquared*cos(calc);
			pointY[i] = pointDistSquared*sin(calc);
		}
		float wLowerBound = pointX[0], wUpperBound = pointX[0], hLowerBound = pointY[0], hUpperBound = pointY[0];
		for (short i = 0; i < 4; i++) {
			if (pointX[i] < wLowerBound) wLowerBound = pointX[i];
			if (pointX[i] > wUpperBound) wUpperBound = pointX[i];
			if (pointY[i] < hLowerBound) hLowerBound = pointY[i];
			if (pointY[i] > hUpperBound) hUpperBound = pointY[i];
		}
		zRotatedWidth_ = sqrt(wUpperBound-wLowerBound);
		zRotatedHeight_ = sqrt(hUpperBound-hLowerBound);
	}
}

void Object::scale(float xAmount, float yAmount, float zAmount, bool relative, bool recalculateDimensions) {
	if (relative) xScale_ += xAmount*compensation(),
			yScale_ += yAmount*compensation(),
			zScale_ += zAmount*compensation();
		else xScale_ = xAmount, yScale_ = yAmount, zScale_ = zAmount;
	width_ *= xScale_, height_*= yScale_, originX *= xScale_, originY *= yScale_;
	if (recalculateDimensions) calcZRotatedDimensions();
}

float Object::setXScale(float amount, bool relative) {
	if (relative) xScale_ += amount*compensation(); else xScale_ = amount;
	return xScale_;
}

float Object::setYScale(float amount, bool relative) {
	if (relative) yScale_ += amount*compensation(); else yScale_ = amount;
	return yScale_;
}

float Object::setZScale(float amount, bool relative) {
	if (relative) zScale_ += amount*compensation(); else zScale_ = amount;
	return zScale_;
}

float Object::xScale() {
	return xScale_;
}

float Object::yScale() {
	return yScale_;
}

float Object::zScale() {
	return zScale_;
}

void Object::rotate(float xAmount, float yAmount, float zAmount, bool relative, bool recalculateDimensions) {
	if (relative) xRotation_ += xAmount*compensation(), yRotation_ += yAmount*compensation(),
			zRotation_ += zAmount*compensation();
		else xRotation_ = xAmount, yRotation_ = yAmount, zRotation_ = zAmount;
	if (xRotation_ >= 360.0f) xRotation_ -= 360.0f; else if (xRotation_ < 0.0f) xRotation_ += 360.0f;
	if (yRotation_ >= 360.0f) yRotation_ -= 360.0f; else if (yRotation_ < 0.0f) yRotation_ += 360.0f;
	if (zRotation_ >= 360.0f) zRotation_ -= 360.0f; else if (zRotation_ < 0.0f) zRotation_ += 360.0f;
	if (recalculateDimensions) calcZRotatedDimensions();
}

float Object::rotate(float amount, bool relative, bool recalculateDimensions) {
	if (relative) zRotation_ += amount*compensation(); else zRotation_ = amount;
	if (zRotation_ >= 360.0f) zRotation_ -= 360.0f; else if (zRotation_ < 0.0f) zRotation_ += 360.0f;
	if (recalculateDimensions) calcZRotatedDimensions();
	return zRotation_;
}

float Object::setXRotation(float amount, bool relative) {
	if (relative) xRotation_ += amount*compensation(); else xRotation_ = amount;
	return xRotation_;
}

float Object::setYRotation(float amount, bool relative) {
	if (relative) yRotation_ += amount*compensation(); else yRotation_ = amount;
	return yRotation_;
}

float Object::setZRotation(float amount, bool relative) {
	if (relative) zRotation_ += amount*compensation(); else zRotation_ = amount;
	return zRotation_;
}

float Object::xRotation() {
	return xRotation_;
}

float Object::yRotation() {
	return yRotation_;
}

float Object::zRotation() {
	return zRotation_;
}

float Object::setAlpha(float amount, bool relative) {
	if (relative) alpha_ += amount*compensation(); else alpha_ = amount;
	return alpha_;
}

float Object::alpha() {
	return alpha_;
}

void Object::setCurrentAnimation(unsigned animationId, int boneId, bool withChildren) {
	if (model_->bones_.size() == 0) return;

	if (boneId < 0) boneId = 0;
	currentAnimationId[boneId] = animationId;

	if (withChildren && (model_->bones_[boneId]->child.size() > 0)) {
		for (unsigned i = 0; i < model_->bones_[boneId]->child.size(); i++)
			setCurrentAnimation(animationId, model_->bones_[i]->id, true);
	}
}

unsigned Object::currentAnimation(int boneId) {
	if (model_->bones_.size() == 0) return 0;
	return (boneId < 0) ? currentAnimationId.front() : currentAnimationId[boneId];
}

void Object::setFrame(float newFrame, bool relative, int boneId, bool withChildren) {
	if (!hasModel_) {
		if (relative) frame_.front() += newFrame*compensation(); else frame_.front() = newFrame;
		return;
	}
	if (model_->bones_.size() == 0) return;

	if (boneId < 0) boneId = 0;
	if (relative) frame_[boneId] += newFrame*compensation(); else frame_[boneId] = newFrame;
	if (hasModel_) {
		if (model_->bones_.size() > 0) {
			while (frame_[boneId] > model_->bones_[boneId]->animations[currentAnimationId[boneId]].length)
				frame_[boneId] -= model_->bones_[boneId]->animations[currentAnimationId[boneId]].length;
			while (frame_[boneId] < 1.0f)
				frame_[boneId] += (model_->bones_[boneId]->animations[currentAnimationId[boneId]].length-1);
		}
	} else {
		while (frame_.front() > sprite_->frames) frame_.front() -= (float)sprite_->frames;
		while (frame_.front() < 1.0f) frame_.front() += float(sprite_->frames)-1.0f;
	}

	if (withChildren && (model_->bones_[boneId]->child.size() > 0)) {
		for (unsigned i = 0; i < model_->bones_[boneId]->child.size(); i++)
			setFrame(newFrame, relative, model_->bones_[boneId]->child[i]->id, true);
	}
}

float Object::frame(int boneId) {
	if (model_->bones_.size() == 0) return 0;
	return (boneId < 0) ? frame_.front() : frame_[boneId];
}

void Object::draw(bool skipAnimation) {
	if (hasModel_) {
		if (model_ != NULL) model_->draw(this, skipAnimation);
	} else {
		if (sprite_ != NULL) sprite_->draw(this);
	}
}

bool Object::mouseOverBox() {
	if (!hasModel_) {
		vec2 vertex[4] = {
				vec2(x_-originX, y_-originY),
				vec2(x_-originX, y_-originY+height_),
				vec2(x_-originX+width_, y_-originY+height_),
				vec2(x_-originX+width_, y_-originY)
		};

		if ((zRotation_ != 0.0f) && (zRotation_ != 180.0f))
			for (short i = 0; i < 4; i++) vertex[i] *= get2dRotationMatrix(zRotation_);

		return vec2(mouseX(), mouseY()).polygonCollision(4, vertex);
	}
	return false;
}

bool Object::roughMouseOverBox() {
	if (!hasModel_) {
		if (mouseX() < x_-originX) return false;
		if (mouseX() > x_-originX+zRotatedWidth_) return false;
		if (mouseY() < y_-originY) return false;
		if (mouseY() > y_-originY+zRotatedHeight_) return false;
	}
	return true;
}

bool Object::mouseOverCircle() {
	if (!hasModel_) {
		float radius = (width_ > height_) ? width_ : height_,
			mouseXDist = float(mouseX())-x_, mouseYDist = float(mouseY())-y_,
			mouseDist = (mouseXDist*mouseXDist)+(mouseYDist*mouseYDist);

		if (mouseDist <= (radius/2)*(radius/2)) return true;
	}
	return false;
}

bool Object::boxCollision(Object * other, bool allStages) {
	if (!hasModel_ && !other->hasModel_) {
		vec2 box[4] = {
				vec2(x_-originX, y_-originY),
				vec2(x_-originX, y_-originY+height_),
				vec2(x_-originX+width_, y_-originY+height_),
				vec2(x_-originX+width_, y_-originY)
		};

		vec2 otherBox[4] = {
				vec2(other->x_-other->originX, other->y_-other->originY),
				vec2(other->x_-other->originX, other->y_-other->originY+other->height_),
				vec2(other->x_-other->originX+other->width_, other->y_-other->originY+other->height_),
				vec2(other->x_-other->originX+other->width_, other->y_-other->originY)
		};

		if ((zRotation_ != 0.0f) && (zRotation_ != 180.0f))
			for (short i = 0; i < 4; i++) box[i] *= get2dRotationMatrix(zRotation_);
		if ((other->zRotation_ != 0.0f) && (other->zRotation_ != 180.0f))
			for (short i = 0; i < 4; i++) box[i] *= get2dRotationMatrix(other->zRotation_);

		for (short i = 0; i < 4; i++) if (box[i].polygonCollision(4, otherBox)) return true;

		if (allStages) {
			for (short i = 0; i < 4; i++) if (otherBox[i].polygonCollision(4, box)) return true;
		}
	}
	return false;
}

bool Object::roughBoxCollision(Object * other) {
	if (!hasModel_ && !other->hasModel_) {
		if (x_+zRotatedWidth_-originX < other->x_-other->originX) return false;
		if (other->x_+other->zRotatedWidth_-other->originX < x_-originX) return false;
		if (y_+zRotatedHeight_-originY < other->y_-other->originY) return false;
		if (other->y_+other->zRotatedHeight_-other->originY < y_-originY) return false;
	}
	return true;
}

bool Object::circleCollision(Object * other) {
	if (!hasModel_) {
		float radius = (width_ > height_) ? width_ : height_,
			otherRadius  = (other->width_ > other->height_) ? other->width_ : other->height_,
			xDist = x_-other->x_, yDist = y_-other->y_, dist = (xDist*xDist)+(yDist*yDist);

		if (dist <= (radius+otherRadius)*(radius+otherRadius)) return true;
	}
	return false;
}

Object * object(const string & searchName) {
	int letter = numCharInAlphabet(searchName[0]);
	Object * returnObject = NULL;
	if (allObjects[letter].size() > 0) {
		for (unsigned int i = 0; i < allObjects[letter].size(); i++) {
			if (allObjects[letter][i]->name() == searchName) {
				returnObject = allObjects[letter][i];
				break;
			}
		}
	}
	return returnObject;
}

Object * addObject(const string & newName, float destX, float destY, float destZ, Sprite * newSprite) {
	int letter = numCharInAlphabet(newName[0]);
	Object * newObject = new Object(newName, destX, destY, destZ, newSprite);
	allObjects[letter].push_back(newObject);
	return newObject;
}

Object * addObject(const string & newName, float destX, float destY, float destZ, Model * newModel) {
	int letter = numCharInAlphabet(newName[0]);
	Object * newObject = new Object(newName, destX, destY, destZ, newModel);
	allObjects[letter].push_back(newObject);
	return newObject;
}

void destroyObject(const string & searchName) {
	int letter = numCharInAlphabet(searchName[0]);
	if (allObjects[letter].size() > 0) {
		for (unsigned int i = 0; i < allObjects[letter].size(); i++) {
			if (allObjects[letter][i]->name() == searchName) {
				delete allObjects[letter][i];
				allObjects[letter].erase(allObjects[letter].begin()+i);
				break;
			}
		}
	}
}

void destroyAllObjects() {
	for (int i = 0; i < 27; i++) {
		if (allObjects[i].size() > 0) {
			for (unsigned int j = 0; j < allObjects[i].size(); j++) delete allObjects[i][j];
			allObjects[i].clear();
		}
	}
}

}
