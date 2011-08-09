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
#include <SDL/SDL_video.h>
#include <GL/gl.h>
#include "../../headers/classes/Object.h"
#include "../../headers/classes/Sprite.h"
#include "../../headers/classes/Model.h"
#include "../../headers/Display.h"
#include "../../headers/Utils.h"
#include "../../headers/Input.h"
using namespace SuperMaximo;

vector<Object*> allObjects[27];

namespace SuperMaximo {

Object::Object(string newName, float destX, float destY, float destZ, Sprite * newSprite) {
	name_ = newName, x_ = destX, y_ = destY, z_ = destZ, sprite_ = newSprite, frame_ = 1.0f, currentAnimationId = nextAnimationId = 0;
	xRotation_ = 0.0f, yRotation_ = 0.0f, zRotation_ = 0.0f, xScale_ = 1.0f, yScale_ = 1.0f, zScale_ = 1.0f, alpha_ = 1.0f, hasModel_ = false;
	if (sprite_ != NULL) {
		width_ = sprite_->rect.w, height_ = sprite_->rect.h, originX = sprite_->originX, originY = sprite_->originY;
	}
	zRotatedWidth_ = width_, zRotatedHeight_ = height_;
	boundShader_ = NULL;
	customDrawFunction = NULL;
	//fakeKeyFrame1 = fakeKeyFrame2 = NULL;
	//interpolating = false;
}

Object::Object(string newName, float destX, float destY, float destZ, Model * newModel) {
	name_ = newName, x_ = destX, y_ = destY, z_ = destZ, model_ = newModel, currentAnimationId = nextAnimationId = 0;
	xRotation_ = 0.0f, yRotation_ = 0.0f, zRotation_ = 0.0f, xScale_ = 1.0f, yScale_ = 1.0f, zScale_= 1.0f, alpha_ = 1.0f, hasModel_ = true;
	frame_ = 1.0f;
	boundShader_ = NULL;
	customDrawFunction = NULL;
	//fakeKeyFrame1 = new keyFrame;
	//fakeKeyFrame2 = new keyFrame;
	//interpolating = false;
}

Object::~Object() {
	//if (fakeKeyFrame1 != NULL) delete fakeKeyFrame1;
	//if (fakeKeyFrame2 != NULL) delete fakeKeyFrame2;
}

string Object::name() {
	return name_;
}

void Object::setSprite(Sprite * newSprite) {
	sprite_ = newSprite;
	if (sprite_ != NULL) width_ = sprite_->rect.w, height_ = sprite_->rect.h, originX = sprite_->originX, originY = sprite_->originY;
	hasModel_ = false;
	//if (fakeKeyFrame1 != NULL) delete fakeKeyFrame1;
	//if (fakeKeyFrame2 != NULL) delete fakeKeyFrame2;
}

Sprite * Object::sprite() {
	return sprite_;
}

void Object::setModel(Model * newModel) {
	model_ = newModel;
	hasModel_ = true;
	//if (fakeKeyFrame1 == NULL) fakeKeyFrame1 = new keyFrame;
	//if (fakeKeyFrame2 == NULL) fakeKeyFrame2 = new keyFrame;
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
	if (relative) x_ += xAmount*compensation(), y_ += yAmount*compensation(), z_ += zAmount*compensation(); else x_ = xAmount, y_ = yAmount, z_ = zAmount;
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
	if (relative) xScale_ += xAmount*compensation(), yScale_ += yAmount*compensation(), zScale_ += zAmount*compensation(); else
		xScale_ = xAmount, yScale_ = yAmount, zScale_ = zAmount;
	width_ *= xScale_, height_*= yScale_, originX *= xScale_, originY *= yScale_;
	if (recalculateDimensions) calcZRotatedDimensions();
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
	if (relative) xRotation_ += xAmount*compensation(), yRotation_ += yAmount*compensation(), zRotation_ += zAmount*compensation(); else
		xRotation_ = xAmount, yRotation_ = yAmount, zRotation_ = zAmount;
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

void Object::setCurrentAnimation(unsigned animationId) {
	currentAnimationId = animationId;
}

unsigned Object::currentAnimation() {
	return currentAnimationId;
}

void Object::setFrame(float newFrame, bool relative) {
	if (relative) frame_ += newFrame*compensation(); else frame_ = newFrame;
	if (hasModel_) {
		if (model_->bones_.size() > 0) {
			while (frame_ > model_->bones_.front()->animations[currentAnimationId].length) frame_ -= model_->bones_.front()->animations[currentAnimationId].length;
			while (frame_ < 1.0f) frame_ += (model_->bones_.front()->animations[currentAnimationId].length-1);
		}
	} else {
		while (frame_ > sprite_->frames) frame_ -= sprite_->frames;
		while (frame_ < 1.0f) frame_ += sprite_->frames-1;
	}
}

float Object::frame() {
	return frame_;
}

/*void Object::animate(unsigned start, unsigned finish, unsigned animationId) {
	if (hasModel_) {
		if (model_->bones_.size() > 0) {
			currentAnimationId = animationId;
			while (currentAnimationId >= model_->bones_.front()->animations.size()) currentAnimationId -= model_->bones_.front()->animations.size();
			if (start < finish) {
				frame_ += compensation();
				if (frame_ > finish) frame_ = start+((frame_-1.0f)-finish);
			} else if (start > finish) {
				frame_ -= compensation();
				if (frame_ < 0.0f) frame_ = finish+(frame_+1.0f); else if (frame_ < start) frame_ = finish+((frame_-start)+1.0f);
			}
		}
	} else {
		if (start < finish) {
			frame_ += compensation();
			if (frame_ > finish) frame_ = start+((frame_-1.0f)-finish); else if (frame_ > sprite_->frames) frame_ = start+((frame_-1.0f)-sprite_->frames);
		} else if (start > finish) {
			frame_ -= compensation();
			if (frame_ < 0.0f) frame_ = finish+(frame_+1.0f); else if (frame_ < start) frame_ = finish+((frame_-start)+1.0f);
		}
	}
}*/
/*
void Object::setInterpolation(int startFrame, unsigned animation1Id, int endFrame, unsigned animation2Id, unsigned numFramesToTake) {
	if (hasModel_) {
		interpolating = true;
		fakeKeyFrame1->step = 0;
		fakeKeyFrame1->boneData = model_->animations[animation1Id].frames[0].boneData;
		fakeKeyFrame2->step = numFramesToTake-1;
		fakeKeyFrame2->boneData = model_->animations[animation2Id].frames[0].boneData;

		for (short frameCount = 0; frameCount < 2; frameCount++) {
			int * frameToUse, animationId;
			keyFrame * fakeKeyFrame;
			if (frameCount) {
				frameToUse = &endFrame;
				fakeKeyFrame = fakeKeyFrame2;
				animationId = animation2Id;
			} else {
				frameToUse = &startFrame;
				fakeKeyFrame = fakeKeyFrame1;
				animationId = animation1Id;
			}
			unsigned currentKeyFrame = 0;
			while (*frameToUse > (int)model_->animations[animationId].frames[currentKeyFrame+1].step) {
				currentKeyFrame++;
				if (currentKeyFrame >= model_->animations[animationId].frames.size()) {
					currentKeyFrame = 0;
					*frameToUse -= model_->animations[animationId].frames.back().step;
					break;
				}
			}

			keyFrame * nextKeyFrame, * thisKeyFrame = &model_->animations[animationId].frames[currentKeyFrame];
			if (currentKeyFrame+1 < model_->animations[animationId].frames.size()) nextKeyFrame = &model_->animations[animationId].frames[currentKeyFrame+1];
				else nextKeyFrame = &model_->animations[animationId].frames.front();

			int tempFrame = *frameToUse-thisKeyFrame->step;
			int stepDiff = nextKeyFrame->step-thisKeyFrame->step;

			for (unsigned i = 0; i < model_->bones_.size(); i++) {
				float diff, diff1 = nextKeyFrame->boneData[i].xRot-thisKeyFrame->boneData[i].xRot, diff2 = 360.0f-nextKeyFrame->boneData[i].xRot;
				if (abs(diff1) < abs(diff2)) diff = diff1; else diff = diff2;
				fakeKeyFrame->boneData[i].xRot = thisKeyFrame->boneData[i].xRot+((diff/(float)stepDiff)*(float)tempFrame);

				diff1 = nextKeyFrame->boneData[i].yRot-thisKeyFrame->boneData[i].yRot, diff2 = 360.0f-nextKeyFrame->boneData[i].yRot;
				if (abs(diff1) < abs(diff2)) diff = diff1; else diff = diff2;
				fakeKeyFrame->boneData[i].yRot = thisKeyFrame->boneData[i].yRot+((diff/(float)stepDiff)*(float)tempFrame);

				diff1 = nextKeyFrame->boneData[i].zRot-thisKeyFrame->boneData[i].zRot, diff2 = 360.0f-nextKeyFrame->boneData[i].zRot;
				if (abs(diff1) < abs(diff2)) diff = diff1; else diff = diff2;
				fakeKeyFrame->boneData[i].zRot = thisKeyFrame->boneData[i].zRot+((diff/(float)stepDiff)*(float)tempFrame);
			}
		}
	}
}

void Object::setInterpolation() {
	interpolating = false;
}
*/
void Object::draw(bool skipAnimation, bool skipHitboxes) {
	if (hasModel_) {
		if (model_ != NULL) model_->draw(this, skipAnimation, skipHitboxes);
	} else {
		if (sprite_ != NULL) sprite_->draw(this);
	}
}

bool Object::mouseOverBox() {
	if (!hasModel_) {
		float mouseX_, mouseY_;
		if (zRotation_ != 0.0f) {
			float mouseXDist = (float)mouseX()-x_, mouseYDist = (float)mouseY()-y_;
			float mouseDistSquared = (mouseXDist*mouseXDist)+(mouseYDist*mouseYDist);
			float angle = atan2(mouseYDist, mouseXDist);
			mouseX_ = (mouseDistSquared*cos(angle-((zRotation_*M_PI)/180.0f)))+x_;
			mouseY_ = (mouseDistSquared*sin(angle-((zRotation_*M_PI)/180.0f)))+y_;
		} else {
			mouseX_ = mouseX()*mouseX();
			mouseY_ = mouseY()*mouseX();
		}
		if ((mouseX_ >= (x_-originX)*(x_-originX)) && (mouseX_ <= (x_-originX+width_)*(x_-originX+width_))) {
			if ((mouseY_ >= (y_-originY)*(y_-originY)) && (mouseY_ <= (y_-originY+height_)*(y_-originY+height_))) return true;
		}
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

bool Object::mouseOverCircle(float extraX, float extraY, float extraZ) {
	if (!hasModel_) {
		float mouseXDist = (float)mouseX()-(x_+extraX), mouseYDist = (float)mouseY()-(y_+extraY);
		float mouseDist = (mouseXDist*mouseXDist)+(mouseYDist*mouseYDist);
		if (mouseDist <= (width_/2)*(width_/2)) return true;
	}
	return false;
}

bool Object::boxCollision(Object * other, bool allStages) {
	if (!hasModel_ && !other->hasModel_) {
		for (int i = 0; i < 4; i++) {
			float pointX, pointY, tempPointX, tempPointY;
			switch (i) {
			case 0:
				tempPointX = -other->originX;
				tempPointY = -other->originY;
				break;
			case 1:
				tempPointX = -other->originX;
				tempPointY = -other->originY+other->height_;
				break;
			case 2:
				tempPointX = -other->originX+other->width_;
				tempPointY = -other->originY+other->height_;
				break;
			case 3:
				tempPointX = -other->originX+other->width_;
				tempPointY = -other->originY;
				break;
			}
			if (other->zRotation_ == 0.0f) {
				tempPointX += other->x_;
				tempPointY += other->y_;
			} else {
				float tempPointDistSquared = (tempPointX*tempPointX)+(tempPointY*tempPointY);
				float tempAngle = atan2(tempPointY, tempPointX);
				float calc = tempAngle-((other->zRotation_*M_PI)/180.0f);
				tempPointX = (tempPointDistSquared*cos(calc))+other->x_;
				tempPointY = (tempPointDistSquared*sin(calc))+other->y_;
			}
			if (zRotation_ == 0.0f) {
				pointX = tempPointX;
				pointY = tempPointY;
			} else {
				float pointXDist = tempPointX-x_, pointYDist = tempPointY-y_;
				float pointDistSquared = (pointXDist*pointXDist)+(pointYDist*pointYDist);
				float angle = atan2(pointYDist, pointXDist);
				float calc = angle-((zRotation_*M_PI)/180.0f);
				pointX = (pointDistSquared*cos(calc))+x_;
				pointY = (pointDistSquared*sin(calc))+y_;
			}
			if ((pointX >= (x_-originX)*(x_-originX)) && (pointX <= ((x_-originX)+width_)*((x_-originX)+width_))) {
				if ((pointY >= (y_-originY)*(y_-originY)) && (pointY <= ((y_-originY)+height_)*((y_-originY)+height_))) return true;
			}
		}
		if (!allStages) {
			if ((other->x_ >= x_-originX) && (other->x_ <= (x_-originX)+width_)) {
				if ((other->y_ >= y_-originY) && (other->y_ <= (y_-originY)+height_)) return true;
			}
			return false;
		}
		for (int i = 0; i < 5; i++) {
			float pointX, pointY, tempPointX, tempPointY;
			if (i < 4) {
				switch (i) {
				case 0:
					tempPointX = -originX;
					tempPointY = -originY;
					break;
				case 1:
					tempPointX = -originX;
					tempPointY = -originY+height_;
					break;
				case 2:
					tempPointX = -originX+width_;
					tempPointY = -originY+height_;
					break;
				case 3:
					tempPointX = -originX+width_;
					tempPointY = -originY;
					break;
				}
				if (zRotation_ == 0.0f) {
					tempPointX += x_;
					tempPointY += y_;
				} else {
					float tempPointDistSquared = (tempPointX*tempPointX)+(tempPointY*tempPointY);
					float tempAngle = atan2(tempPointY, tempPointX);
					float calc = tempAngle-((zRotation_*M_PI)/180.0f);
					tempPointX = (tempPointDistSquared*cos(calc))+x_;
					tempPointY = (tempPointDistSquared*sin(calc))+y_;
				}
				if (other->zRotation_ == 0.0f) {
					pointX = tempPointX;
					pointY = tempPointY;
				} else {
					float pointXDist = tempPointX-other->x_, pointYDist = tempPointY-other->y_;
					float pointDistSquared = (pointXDist*pointXDist)+(pointYDist*pointYDist);
					float angle = atan2(pointYDist, pointXDist);
					float calc = angle-((other->zRotation_*M_PI)/180.0f);
					pointX = (pointDistSquared*cos(calc))+other->x_;
					pointY = (pointDistSquared*sin(calc))+other->y_;
				}
			} else {
				pointX = x_;
				pointY = y_;
			}
			if ((pointX >= (other->x_-other->originX)*(other->x_-other->originX)) &&
					(pointX <= ((other->x_-other->originX)+other->width_)*(other->x_-other->originX)+other->width_)) {
				if ((pointY >= (other->y_-other->originY)*(other->y_-other->originY)) &&
						(pointY <= ((other->y_-other->originY)+other->height_)*((other->y_-other->originY)+other->height_))) return true;
			}
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

bool Object::circleCollision(Object * other, float extraX1, float extraY1, float extraZ1, float extraX2, float extraY2, float extraZ2) {
	return false;
}

bool Object::roughHitboxCollision(bone * bone1, bone * bone2) {
	if ((bone1 != NULL) && (bone2 != NULL)) {
		if (bone1->hitbox.x+bone1->hitbox.rl < bone2->hitbox.x) return false;
		if (bone2->hitbox.x+bone2->hitbox.rl < bone1->hitbox.x) return false;

		if (bone1->hitbox.y+bone1->hitbox.rh < bone2->hitbox.y) return false;
		if (bone2->hitbox.y+bone2->hitbox.rh < bone1->hitbox.y) return false;

		if (bone1->hitbox.z+bone1->hitbox.rw < bone2->hitbox.z) return false;
		if (bone2->hitbox.z+bone2->hitbox.rw < bone1->hitbox.z) return false;
	}
	return true;
}

bool Object::hitBoxCollision(bone * bone1, bone * bone2) {
	return false;
}

bool Object::roughModelCollision(Object * other, int hitboxId, int hitboxOtherId) {
	if (hasModel_ && other->hasModel_) {
		if (hitboxId < 0) {
			if (hitboxOtherId < 0) {
				for (unsigned i = 0; i < model_->bones_.size(); i++) {
					for (unsigned j = 0; j < other->model_->bones_.size(); j++) {
						if (roughHitboxCollision(model_->bones_[i], other->model_->bones_[j])) return true;
					}
				}
			} else {
				for (unsigned i = 0; i < model_->bones_.size(); i++) {
					if (roughHitboxCollision(model_->bones_[i], other->model_->bones_[hitboxOtherId])) return true;
				}
			}
		} else {
			if (hitboxOtherId < 0) {
				for (unsigned i = 0; i < other->model_->bones_.size(); i++) {
					if (roughHitboxCollision(model_->bones_[hitboxId], other->model_->bones_[i])) return true;
				}
			} else {
				if (roughHitboxCollision(model_->bones_[hitboxId], other->model_->bones_[hitboxOtherId])) return true;
			}
		}
	}
	return false;
}

bool Object::modelCollision(Object * other, int hitboxId, int hitboxOtherId) {
	if (hasModel_ && other->hasModel_) {

	}
	return false;
}

Object * object(string searchName) {
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

Object * addObject(string newName, float destX, float destY, float destZ, Sprite * newSprite) {
	int letter = numCharInAlphabet(newName[0]);
	Object * newObject = new Object(newName, destX, destY, destZ, newSprite);
	allObjects[letter].push_back(newObject);
	return newObject;
}

Object * addObject(string newName, float destX, float destY, float destZ, Model * newModel) {
	int letter = numCharInAlphabet(newName[0]);
	Object * newObject = new Object(newName, destX, destY, destZ, newModel);
	allObjects[letter].push_back(newObject);
	return newObject;
}

void destroyObject(string searchName) {
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
