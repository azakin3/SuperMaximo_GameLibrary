//============================================================================
// Name        : Model.cpp
// Author      : Max Foster
// Created on  : 30 May 2011
// Version     : 1.0
// Copyright   : http://creativecommons.org/licenses/by/3.0/
// Description : SuperMaximo GameLibrary 3D Model class
//============================================================================

#define GL3_PROTOTYPES 1
#include <GL3/gl3.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
using namespace std;
#include <SDL/SDL_image.h>
#include "../../headers/classes/Model.h"
#include "../../headers/classes/Object.h"
#include "../../headers/classes/Shader.h"
#include "../../headers/Display.h"
#include "../../headers/Utils.h"
using namespace SuperMaximo;

vector<Model*> allModels[27];

namespace SuperMaximo {
/*
void keyFrame::set(Model * model) {
	for (unsigned i = 0; i < boneData.size(); i++) {
		model->bones_[boneData[i].boneId]->xRot = boneData[i].xRot;
		model->bones_[boneData[i].boneId]->yRot = boneData[i].yRot;
		model->bones_[boneData[i].boneId]->zRot = boneData[i].zRot;
	}
}

Model::normal Model::normal::operator- (normal const & other) {
	normal returnNormal;
	returnNormal.x = x-other.x;
	returnNormal.y = y-other.y;
	returnNormal.z = z-other.z;
	return returnNormal;
}*/

Model::vertex Model::vertex::operator- (vertex const & other) {
	vertex returnVertex;
	returnVertex.x = x-other.x;
	returnVertex.y = y-other.y;
	returnVertex.z = z-other.z;
	return returnVertex;
}

vec3 Model::triangle::surfaceNormal() {
	vertex u, v;
	vec3 normal_;
	u = coords[1]-coords[0];
	v = coords[2]-coords[0];
	normal_.x = (u.y*v.z)-(u.z*v.y);
	normal_.y = (u.z*v.x)-(u.x*v.z);
	normal_.z = (u.x*v.y)-(u.y*v.x);

	float len = sqrt((normal_.x*normal_.x)+(normal_.y*normal_.y)+(normal_.z*normal_.z));
	if (len == 0) len = 1;
	normal_.x /= len;
	normal_.y /= len;
	normal_.z /= len;

	return normal_;
}

void bone::box::init() {
	x = y = z = l = w = h = xRot = yRot = zRot = 0.0f;
}

Model::Model(string newName, string path, string fileName, unsigned framerate, bufferUsageEnum bufferUsage, void (*customBufferFunction)(GLuint*, Model*, void*),
		void * customData) {
	name_ = newName;
	boundShader_ = NULL;
	framerate_ = framerate;
	if (lowerCase(leftStr(rightStr(fileName, 3), 2)) == "sm") {
		switch (rightStr(fileName, 1)[0]) {
		case 'o': loadSmo(path, fileName, bufferUsage); break;
		case 'm': loadSmm(path, fileName, bufferUsage); break;
		}
	} else if (lowerCase(rightStr(fileName, 3)) == "obj") loadObj(path, fileName, bufferUsage, customBufferFunction, customData);
}

Model::~Model() {
	glDeleteTextures(1, &texture);
	for (unsigned i = 0; i < bones_.size(); i++) delete bones_[i];
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
}

void Model::loadObj(string path, string fileName, bufferUsageEnum bufferUsage, void (*customBufferFunction)(GLuint*, Model*, void*), void * customData) {
	vector<string> objText, mtlText;
	vector<vertex> vertices, texCoord;
	ifstream file;
	file.open((path+fileName).c_str());
	if (file.is_open()) {
		while (!file.eof()) {
			string tempStr;
			getline(file, tempStr);
			objText.push_back(tempStr);
		}
		file.close();
	} else {
		cout << "File " << fileName << " could not be loaded" << endl;
		return;
	}

	for (unsigned i = 0; i < objText.size(); i++) {
		while ((leftStr(objText[i], 1) == " ") || (leftStr(objText[i], 1) == "\t")) rightStr(&objText[i], objText[i].size()-1);
		while ((rightStr(objText[i], 1) == " ") || (rightStr(objText[i], 1) == "\t") || (*(rightStr(objText[i], 1).c_str()) == 13)) leftStr(&objText[i], objText[i].size()-1);
	}
	string mtlFileName;
	for (unsigned i = 0; i < objText.size(); i++) {
		if (lowerCase(leftStr(objText[i], 6)) == "mtllib") {
			mtlFileName = rightStr(objText[i], objText[i].size()-7);
			break;
		}
	}

	file.open((path+mtlFileName).c_str());
	if (file.is_open()) {
		while (!file.eof()) {
			string tempStr;
			getline(file, tempStr);
			mtlText.push_back(tempStr);
		}
		file.close();
	}

	for (unsigned i = 0; i < mtlText.size(); i++) {
		while ((leftStr(mtlText[i], 1) == " ") || (leftStr(mtlText[i], 1) == "\t")) rightStr(&mtlText[i], mtlText[i].size()-1);
		while ((rightStr(mtlText[i], 1) == " ") || (rightStr(mtlText[i], 1) == "\t") || (*(rightStr(mtlText[i], 1).c_str()) == 13)) leftStr(&mtlText[i], mtlText[i].size()-1);
	}
	bool initialised = false;
	unsigned totalMaterials = 0;
	for (unsigned i = 0; i < mtlText.size(); i++) {
		if (lowerCase(leftStr(mtlText[i], 6)) == "newmtl") totalMaterials++;
	}
	for (unsigned i = 0; i < mtlText.size(); i++) {
		if (lowerCase(leftStr(mtlText[i], 6)) == "newmtl") {
			material newMaterial;
			newMaterial.name = rightStr(mtlText[i], mtlText[i].size()-7);
			newMaterial.hasTexture = false;
			//newMaterial.texture = 0;

			string kStr = "";
			for (unsigned j = i+1; j < mtlText.size(); j++) {
				if (lowerCase(leftStr(mtlText[j], 2)) == "ka") {
					kStr = rightStr(mtlText[j], mtlText[j].size()-3);
					break;
				}
			}
			if (kStr == "") kStr = "0.0000 0.0000 0.0000";
			int j = kStr.find(" ");
			newMaterial.ambientColor.r = strtof(leftStr(kStr, j).c_str(), NULL);
			rightStr(&kStr, kStr.size()-(j+1));

			j = kStr.find(" ");
			newMaterial.ambientColor.g = strtof(leftStr(kStr, j).c_str(), NULL);
			rightStr(&kStr, kStr.size()-(j+1));

			newMaterial.ambientColor.b = strtof(kStr.c_str(), NULL);


			kStr = "";
			for (unsigned j = i+1; j < mtlText.size(); j++) {
				if (lowerCase(leftStr(mtlText[j], 2)) == "kd") {
					kStr = rightStr(mtlText[j], mtlText[j].size()-3);
					break;
				}
			}
			if (kStr == "") kStr = "0.0000 0.0000 0.0000";
			j = kStr.find(" ");
			newMaterial.diffuseColor.r = strtof(leftStr(kStr, j).c_str(), NULL);
			rightStr(&kStr, kStr.size()-(j+1));

			j = kStr.find(" ");
			newMaterial.diffuseColor.g = strtof(leftStr(kStr, j).c_str(), NULL);
			rightStr(&kStr, kStr.size()-(j+1));

			newMaterial.diffuseColor.b = strtof(kStr.c_str(), NULL);


			kStr = "";
			for (unsigned j = i+1; j < mtlText.size(); j++) {
				if (lowerCase(leftStr(mtlText[j], 2)) == "ks") {
					kStr = rightStr(mtlText[j], mtlText[j].size()-3);
					break;
				}
			}
			if (kStr == "") kStr = "0.0000 0.0000 0.0000";
			j = kStr.find(" ");
			newMaterial.specularColor.r = strtof(leftStr(kStr, j).c_str(), NULL);
			rightStr(&kStr, kStr.size()-(j+1));

			j = kStr.find(" ");
			newMaterial.specularColor.g = strtof(leftStr(kStr, j).c_str(), NULL);
			rightStr(&kStr, kStr.size()-(j+1));

			newMaterial.specularColor.b = strtof(kStr.c_str(), NULL);


			kStr = "";
			for (unsigned j = i+1; j < mtlText.size(); j++) {
				if (lowerCase(leftStr(mtlText[j], 1)) == "d") {
					kStr = rightStr(mtlText[j], mtlText[j].size()-2);
					break;
				}
			}
			if (kStr == "") kStr = "1.0000";
			newMaterial.alpha = strtof(kStr.c_str(), NULL);

			kStr = "";
			for (unsigned j = i+1; j < mtlText.size(); j++) {
				if (lowerCase(leftStr(mtlText[j], 2)) == "ns") {
					kStr = rightStr(mtlText[j], mtlText[j].size()-3);
					break;
				}
			}
			if (kStr == "") kStr = "0.0000";
			newMaterial.shininess = strtof(kStr.c_str(), NULL);

			bool texPresent = false;
			newMaterial.fileName = "NOFILENAME";
			string texStr;
			for (unsigned j = i+1; j < mtlText.size(); j++) {
				if (lowerCase(leftStr(mtlText[j], 6)) == "map_kd") {
					texStr = rightStr(mtlText[j], mtlText[j].size()-7);
					texPresent = true;
					break;
				} else if (lowerCase(leftStr(mtlText[j], 6)) == "newmtl") break;
			}

			if (texPresent) {
				SDL_Surface * image = IMG_Load((path+texStr).c_str());
				GLenum textureFormat;
				if (image == NULL) cout << "Could not load image " << path+texStr << endl; else {
					newMaterial.fileName = texStr;
					if (image->format->BytesPerPixel == 4) {
						if (image->format->Rmask == 0x000000ff) textureFormat = GL_RGBA; else textureFormat = GL_BGRA;
					} else {
						if (image->format->Rmask == 0x000000ff) textureFormat = GL_RGB; else textureFormat = GL_BGR;
					}
					if (!initialised) {
						initialised = true;
						glGenTextures(1, &texture);
						glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
						glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
						glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
						if (openGlVersion() >= 3.0f) glGenerateMipmap(GL_TEXTURE_2D_ARRAY); else glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_GENERATE_MIPMAP, GL_TRUE);
						glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, image->format->BytesPerPixel, image->w, image->h, totalMaterials, 0, textureFormat, GL_UNSIGNED_BYTE, NULL);
					}
					glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, materials_.size(), image->w, image->h, 1, textureFormat, GL_UNSIGNED_BYTE, image->pixels);
					SDL_FreeSurface(image);
					newMaterial.hasTexture = true;
				}
			}
			materials_.push_back(newMaterial);
		}
	}

	int mtlNum;
	for (unsigned i = 0; i < objText.size(); i++) {
		if (lowerCase(leftStr(objText[i], 1)) == "v") {
			if (lowerCase(leftStr(objText[i], 2)) == "vt") {
				vertex coord;
				string vertStr = rightStr(objText[i], objText[i].size()-3);
				while ((leftStr(vertStr, 1) == " ") || (leftStr(vertStr, 1)) == "\t") rightStr(&vertStr, vertStr.size()-1);

				int j = vertStr.find(" ");
				coord.x  = strtof(leftStr(vertStr, j).c_str(), NULL);
				rightStr(&vertStr, vertStr.size()-(j+1));
				while ((leftStr(vertStr, 1) == " ") || (leftStr(vertStr, 1)) == "\t") rightStr(&vertStr, vertStr.size()-1);

				j = vertStr.find(" ");
				if (j == -1) coord.y = strtof(vertStr.c_str(), NULL);
					else coord.y = strtof(leftStr(vertStr, j).c_str(), NULL);
				texCoord.push_back(coord);
			} else if (lowerCase(leftStr(objText[i], 2)) == "v ") {
				vertex coord;
				string vertStr = rightStr(objText[i], objText[i].size()-2);
				while ((leftStr(vertStr, 1) == " ") || (leftStr(vertStr, 1)) == "\t") rightStr(&vertStr, vertStr.size()-1);

				int j = vertStr.find(" ");
				coord.x = strtof(leftStr(vertStr, j).c_str(), NULL);
				rightStr(&vertStr, vertStr.size()-(j+1));
				while ((leftStr(vertStr, 1) == " ") || (leftStr(vertStr, 1)) == "\t") rightStr(&vertStr, vertStr.size()-1);

				j = vertStr.find(" ");
				coord.y = strtof(leftStr(vertStr, j).c_str(), NULL);
				rightStr(&vertStr, vertStr.size()-(j+1));
				while ((leftStr(vertStr, 1) == " ") || (leftStr(vertStr, 1)) == "\t") rightStr(&vertStr, vertStr.size()-1);

				coord.z = strtof(vertStr.c_str(), NULL);
				vertices.push_back(coord);
			}
		} else {
			if (lowerCase(leftStr(objText[i], 6)) == "usemtl") {
				for (unsigned j = 0; j < materials_.size(); j++) {
					if (materials_[j].name == rightStr(objText[i], objText[i].size()-7)) {
						mtlNum = j;
						break;
					}
				}
			}
			if (lowerCase(leftStr(objText[i], 1)) == "f") {
				triangle newTriangle;
				newTriangle.mtlNum = mtlNum;
				string vertStr = rightStr(objText[i], objText[i].size()-2);

				if (materials_[mtlNum].hasTexture) {
					int j = vertStr.find(" ");
					string myStr = leftStr(vertStr, j);
					rightStr(&vertStr, vertStr.size()-(j+1));
					j = myStr.find("/");
					int k = atoi((leftStr(myStr, j)).c_str());
					newTriangle.coords[0] = vertices[k-1];
					rightStr(&myStr, myStr.size()-(j+1));
					j = myStr.find("/");
					if (j == -1) j = myStr.size()+1;
					newTriangle.texCoords[0] = texCoord[atoi(leftStr(myStr, j).c_str())-1];

					j = vertStr.find(" ");
					myStr = leftStr(vertStr, j);
					rightStr(&vertStr, vertStr.size()-(j+1));
					j = myStr.find("/");
					k = atoi((leftStr(myStr, j)).c_str());
					newTriangle.coords[1] = vertices[k-1];
					rightStr(&myStr, myStr.size()-(j+1));
					j = myStr.find("/");
					if (j == -1) j = myStr.size()+1;
					newTriangle.texCoords[1] = texCoord[atoi(leftStr(myStr, j).c_str())-1];

					j = vertStr.find("/");
					k = atoi(leftStr(vertStr, j).c_str());
					newTriangle.coords[2] = vertices[k-1];
					myStr = rightStr(vertStr, vertStr.size()-(j+1));
					j = myStr.find(" ");
					if (j== -1) j = myStr.size()+1;
					newTriangle.texCoords[2] = texCoord[atoi(leftStr(myStr, j).c_str())-1];
				} else {
					int j = vertStr.find(" ");
					string myStr = leftStr(vertStr, j);
					rightStr(&vertStr, vertStr.size()-(j+1));
					j = myStr.find("/");
					if (j != -1) leftStr(&myStr, j);
					int k = atoi(myStr.c_str());
					newTriangle.coords[0] = vertices[k-1];

					j = vertStr.find(" ");
					myStr = leftStr(vertStr, j);
					rightStr(&vertStr, vertStr.size()-(j+1));
					j = myStr.find("/");
					if (j != -1) leftStr(&myStr, j);
					k = atoi(myStr.c_str());
					newTriangle.coords[1] = vertices[k-1];

					j = vertStr.find("/");
					if (j != -1) leftStr(&vertStr, j);
					j = atoi(vertStr.c_str());
					newTriangle.coords[2] = vertices[j-1];
				}
				triangles_.push_back(newTriangle);
			}
		}
	}

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	if (customBufferFunction == NULL) initBufferObj(bufferUsage); else (*customBufferFunction)(&vbo, this, customData);
	glBindVertexArray(0);
	for (char i = 0; i < 16; i++) glDisableVertexAttribArray(i);

	vertexCount = triangles_.size()*3;
}

void Model::loadSmm(string path, string fileName, bufferUsageEnum bufferUsage) {
	vector<string> text;
	ifstream file;
	file.open((path+fileName).c_str());
	if (file.is_open()) {
		while (!file.eof()) {
			string tempStr;
			getline(file, tempStr);
			if (leftStr(tempStr, 2) != "//") {
				if (rightStr(tempStr, 1) == "\n") leftStr(&tempStr, tempStr.size()-1);
				text.push_back(tempStr);
			}
		}
		file.close();
	} else {
		cout << "File " << path+fileName << " could not be loaded" << endl;
		return;
	}
	if (text.back() == "") text.pop_back();

	vertexCount = atoi(text.front().c_str())*3;
	unsigned arraySize = vertexCount*24;
	GLfloat data[arraySize];
	for (unsigned i = 0; i < arraySize; i++) data[i] = strtof(text[i+1].c_str(), NULL);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, bufferUsage);

	glVertexAttribPointer(VERTEX_ATTRIBUTE, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*24, 0);
	glVertexAttribPointer(NORMAL_ATTRIBUTE, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*24, (const GLvoid*)(sizeof(GLfloat)*4));
	glVertexAttribPointer(COLOR0_ATTRIBUTE, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*24, (const GLvoid*)(sizeof(GLfloat)*7));
	glVertexAttribPointer(COLOR1_ATTRIBUTE, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*24, (const GLvoid*)(sizeof(GLfloat)*10));
	glVertexAttribPointer(COLOR2_ATTRIBUTE, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*24, (const GLvoid*)(sizeof(GLfloat)*13));
	glVertexAttribPointer(TEXTURE0_ATTRIBUTE, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*24, (const GLvoid*)(sizeof(GLfloat)*16));
	glVertexAttribPointer(EXTRA0_ATTRIBUTE, 1, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*24, (const GLvoid*)(sizeof(GLfloat)*19));
	glVertexAttribPointer(EXTRA1_ATTRIBUTE, 1, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*24, (const GLvoid*)(sizeof(GLfloat)*20));
	glVertexAttribPointer(EXTRA2_ATTRIBUTE, 1, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*24, (const GLvoid*)(sizeof(GLfloat)*21));
	glVertexAttribPointer(EXTRA3_ATTRIBUTE, 1, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*24, (const GLvoid*)(sizeof(GLfloat)*22));
	glVertexAttribPointer(EXTRA4_ATTRIBUTE, 1, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*24, (const GLvoid*)(sizeof(GLfloat)*23));

	glEnableVertexAttribArray(VERTEX_ATTRIBUTE);
	glEnableVertexAttribArray(NORMAL_ATTRIBUTE);
	glEnableVertexAttribArray(COLOR0_ATTRIBUTE);
	glEnableVertexAttribArray(COLOR1_ATTRIBUTE);
	glEnableVertexAttribArray(COLOR2_ATTRIBUTE);
	glEnableVertexAttribArray(TEXTURE0_ATTRIBUTE);
	glEnableVertexAttribArray(EXTRA0_ATTRIBUTE);
	glEnableVertexAttribArray(EXTRA1_ATTRIBUTE);
	glEnableVertexAttribArray(EXTRA2_ATTRIBUTE);
	glEnableVertexAttribArray(EXTRA3_ATTRIBUTE);
	glEnableVertexAttribArray(EXTRA4_ATTRIBUTE);

	glBindVertexArray(0);

	unsigned textureCount = atoi(text[arraySize+1].c_str());
	bool initialised = false;
	for (unsigned i = 0; i < textureCount; i++) {
		SDL_Surface * image = IMG_Load((path+text[arraySize+2+i]).c_str());
		GLenum textureFormat;
		if (image == NULL) cout << "Could not load texture " << path+text[arraySize+2+i] << endl; else {
			if (image->format->BytesPerPixel == 4) {
				if (image->format->Rmask == 0x000000ff) textureFormat = GL_RGBA; else textureFormat = GL_BGRA;
			} else {
				if (image->format->Rmask == 0x000000ff) textureFormat = GL_RGB; else textureFormat = GL_BGR;
			}
			if (!initialised) {
				initialised = true;
				glGenTextures(1, &texture);
				glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
				glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				if (openGlVersion() >= 3.0f) glGenerateMipmap(GL_TEXTURE_2D_ARRAY); else glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_GENERATE_MIPMAP, GL_TRUE);
				glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, image->format->BytesPerPixel, image->w, image->h, textureCount, 0, textureFormat, GL_UNSIGNED_BYTE, NULL);
			}
			glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, image->w, image->h, 1, textureFormat, GL_UNSIGNED_BYTE, image->pixels);
			SDL_FreeSurface(image);
		}
	}

	/*vector<string> text;
	ifstream file;
	file.open(fileName.c_str());
	if (file.is_open()) {
		while (!file.eof()) {
			string tempStr;
			getline(file, tempStr);
			if (rightStr(tempStr, 1) == "\n") leftStr(&tempStr, tempStr.size()-1);
			text.push_back(tempStr);
		}
		file.close();
	} else return;
	if (text.back() == "") text.pop_back();
	triangles_.clear();
	unsigned totalTriangles = atoi(text.front().c_str());
	for (unsigned i = 0; i < totalTriangles; i++) {
		triangle newTriangle;
		for (short j = 0; j < 3; j++) {
			newTriangle.coords[j].x = strtof(text[(i*32)+(j*6)+1].c_str(), NULL);
			newTriangle.coords[j].y = strtof(text[(i*32)+(j*6)+2].c_str(), NULL);
			newTriangle.coords[j].z = strtof(text[(i*32)+(j*6)+3].c_str(), NULL);
			newTriangle.coords[j].normal_.x = strtof(text[(i*32)+(j*6)+4].c_str(), NULL);
			newTriangle.coords[j].normal_.y = strtof(text[(i*32)+(j*6)+5].c_str(), NULL);
			newTriangle.coords[j].normal_.z = strtof(text[(i*32)+(j*6)+6].c_str(), NULL);
		}
		for (short j = 0; j < 3; j++) {
			newTriangle.texCoords[j].x = strtof(text[(i*32)+(j*3)+19].c_str(), NULL);
			newTriangle.texCoords[j].y = strtof(text[(i*32)+(j*3)+20].c_str(), NULL);
			newTriangle.texCoords[j].z = strtof(text[(i*32)+(j*3)+21].c_str(), NULL);
		}
		newTriangle.mtlNum = atoi(text[(i*32)+28].c_str());
		//for (short j = 0; j < 3; j++) newTriangle.sharedCoord[j] = atoi(text[(i*32)+j+29].c_str());
		//if (atoi(text[(i*32)+32].c_str()) < 0) newTriangle.pBone = NULL; else newTriangle.pBone = bones_[atoi(text[(i*32)+32].c_str())];
		triangles_.push_back(newTriangle);
	}
	for (unsigned i = 0; i < materials_.size(); i++) {
		if (materials_[i].hasTexture) glDeleteTextures(1, &materials_[i].texture);
	}
	materials_.clear();
	totalTriangles *= 32;
	unsigned totalMaterials = atoi(text[totalTriangles+1].c_str());
	bool initialised = false;
	for (unsigned i = 0; i < totalMaterials; i++) {
		material newMaterial;
		newMaterial.name = text[totalTriangles+(i*14)+2];
		newMaterial.fileName = text[totalTriangles+(i*14)+3];
		newMaterial.ambientColor.r = strtof(text[totalTriangles+(i*14)+4].c_str(), NULL);
		newMaterial.ambientColor.g = strtof(text[totalTriangles+(i*14)+5].c_str(), NULL);
		newMaterial.ambientColor.b = strtof(text[totalTriangles+(i*14)+6].c_str(), NULL);
		newMaterial.diffuseColor.r = strtof(text[totalTriangles+(i*14)+7].c_str(), NULL);
		newMaterial.diffuseColor.g = strtof(text[totalTriangles+(i*14)+8].c_str(), NULL);
		newMaterial.diffuseColor.b = strtof(text[totalTriangles+(i*14)+9].c_str(), NULL);
		newMaterial.specularColor.r = strtof(text[totalTriangles+(i*14)+10].c_str(), NULL);
		newMaterial.specularColor.g = strtof(text[totalTriangles+(i*14)+11].c_str(), NULL);
		newMaterial.specularColor.b = strtof(text[totalTriangles+(i*14)+12].c_str(), NULL);
		newMaterial.shininess = strtof(text[totalTriangles+(i*14)+13].c_str(), NULL);
		newMaterial.alpha = strtof(text[totalTriangles+(i*14)+14].c_str(), NULL);
		newMaterial.hasTexture = atoi(text[totalTriangles+(i*14)+15].c_str());
		newMaterial.textureId = -1;
		if (newMaterial.hasTexture) {
			SDL_Surface * image = IMG_Load((newMaterial.fileName).c_str());
			GLenum textureFormat;
			if (image == NULL) cout << "Could not load texture " << newMaterial.fileName << endl; else {
				if (image->format->BytesPerPixel == 4) {
					if (image->format->Rmask == 0x000000ff) textureFormat = GL_RGBA; else textureFormat = GL_BGRA;
				} else {
					if (image->format->Rmask == 0x000000ff) textureFormat = GL_RGB; else textureFormat = GL_BGR;
				}
				if (!initialised) {
					initialised = true;
					glGenTextures(1, &texture);
					glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
					glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
					glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_GENERATE_MIPMAP, GL_TRUE);
					glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, image->format->BytesPerPixel, image->w, image->h, totalMaterials, 0, textureFormat, GL_UNSIGNED_BYTE, NULL);
				}
				//glGenTextures(1, &newMaterial.texture);
				//glBindTexture(GL_TEXTURE_2D, newMaterial.texture);
				//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				//glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
				//TODO check for opengl version and use glGenerateMipmap(GL_TEXTURE_2D) after glTexImage2D instead of GL_GENERATE_MIPMAP if >= OpenGL 3
				//glTexImage2D(GL_TEXTURE_2D, 0, image->format->BytesPerPixel, image->w, image->h, 0, textureFormat, GL_UNSIGNED_BYTE, image->pixels);
				glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, image->w, image->h, 1, textureFormat, GL_UNSIGNED_BYTE, image->pixels);
				SDL_FreeSurface(image);
				newMaterial.textureId = i;
			}
		}
		materials_.push_back(newMaterial);
	}
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
	for (unsigned i = 0; i < triangles_.size(); i++) {
		//if (triangles_[i].pBone != NULL) triangles_[i].pBone->triangles.push_back(&(triangles_[i]));
	}*/
}

void Model::loadSms(string fileName) {
	vector<string> text;
	ifstream file;
	file.open(fileName.c_str());
	if (file.is_open()) {
		while (!file.eof()) {
			string tempStr;
			getline(file, tempStr);
			if (leftStr(tempStr, 2) != "//") {
				if (rightStr(tempStr, 1) == "\n") leftStr(&tempStr, tempStr.size()-1);
				text.push_back(tempStr);
			}
		}
		file.close();
	} else {
		cout << "File " << fileName << " could not be loaded" << endl;
		return;
	}
	if (text.back() == "") text.pop_back();

	unsigned boneCount = atoi(text.front().c_str()), line = 1;

	for (unsigned i = 0; i < boneCount; i++) {
		bone * newBone = new bone;

		newBone->id = atoi(text[line].c_str());
		line++;
		newBone->name = text[line];
		line++;
		newBone->x = strtof(text[line].c_str(), NULL);
		line++;
		newBone->y = strtof(text[line].c_str(), NULL);
		line++;
		newBone->z = strtof(text[line].c_str(), NULL);
		line++;
		newBone->endX = strtof(text[line].c_str(), NULL);
		line++;
		newBone->endY = strtof(text[line].c_str(), NULL);
		line++;
		newBone->endZ = strtof(text[line].c_str(), NULL);
		line++;
		int boneParentId = atoi(text[line].c_str());
		if (boneParentId < 0) newBone->parent = NULL; else {
			newBone->parent = bones_[boneParentId];
			bones_[boneParentId]->child.push_back(newBone);
		}
		line++;
		newBone->rotationUpperLimit.x = strtof(text[line].c_str(), NULL);
		line++;
		newBone->rotationUpperLimit.y = strtof(text[line].c_str(), NULL);
		line++;
		newBone->rotationUpperLimit.z = strtof(text[line].c_str(), NULL);
		line++;
		newBone->rotationLowerLimit.x = strtof(text[line].c_str(), NULL);
		line++;
		newBone->rotationLowerLimit.y = strtof(text[line].c_str(), NULL);
		line++;
		newBone->rotationLowerLimit.z = strtof(text[line].c_str(), NULL);
		line++;

		bones_.push_back(newBone);
	}

	/*vector<string> text;
	ifstream file;
	file.open(fileName.c_str());
	if (file.is_open()) {
		while (!file.eof()) {
			string tempStr;
			getline(file, tempStr);
			if (rightStr(tempStr, 1) == "\n") leftStr(&tempStr, tempStr.size()-1);
			text.push_back(tempStr);
		}
		file.close();
	} else return;
	if (text.back() == "") text.pop_back();
	for (unsigned i = 0; i < triangles_.size(); i++) {
		triangles_[i].boneId = -1;
		for (unsigned j = 0; j < bones_.size(); j++) {
			if (triangles_[i].pBone == bones_[j]) {
				triangles_[i].boneId = j;
				break;
			}
		}
	}

	for (unsigned i = 0; i < triangles_.size(); i++) triangles_[i].pBone = NULL;
	for (unsigned i = 0; i < bones_.size(); i++) delete bones_[i];
	bones_.clear();
	unsigned totalBones = atoi(text[0].c_str());
	for (unsigned i = 0; i < totalBones; i++) {
		bones_.push_back(NULL);
		bones_[i] = new bone;
		bone * b = bones_[i];
		//b->triangles.clear();
		b->id = atoi(text[(i*17)+1].c_str());
		b->x = strtof(text[(i*17)+2].c_str(), NULL);
		b->y = strtof(text[(i*17)+3].c_str(), NULL);
		b->z = strtof(text[(i*17)+4].c_str(), NULL);
		b->endX = strtof(text[(i*17)+5].c_str(), NULL);
		b->endY = strtof(text[(i*17)+6].c_str(), NULL);
		b->endZ = strtof(text[(i*17)+7].c_str(), NULL);
		if (atoi(text[(i*17)+8].c_str()) > -1) b->parent = bones_[atoi(text[(i*17)+8].c_str())]; else b->parent = NULL;

		b->hitbox.init();
		b->hitbox.x = strtof(text[(i*17)+9].c_str(), NULL);
		b->hitbox.y = strtof(text[(i*17)+10].c_str(), NULL);
		b->hitbox.z = strtof(text[(i*17)+11].c_str(), NULL);
		b->hitbox.l = strtof(text[(i*17)+12].c_str(), NULL);
		b->hitbox.w = strtof(text[(i*17)+13].c_str(), NULL);
		b->hitbox.h = strtof(text[(i*17)+14].c_str(), NULL);
		b->hitbox.xRot = strtof(text[(i*17)+15].c_str(), NULL);
		b->hitbox.yRot = strtof(text[(i*17)+16].c_str(), NULL);
		b->hitbox.zRot = strtof(text[(i*17)+17].c_str(), NULL);
		b->hitbox.rl = b->hitbox.l;
		b->hitbox.rw = b->hitbox.w;
		b->hitbox.rh = b->hitbox.h;
	}

	for (unsigned i = 0; i < bones_.size(); i++) {
		for (unsigned j = 0; j < bones_.size(); j++) {
			if (bones_[i] == bones_[j]->parent) bones_[i]->child.push_back(bones_[j]);
		}
	}

	for (unsigned i = 0; i < triangles_.size(); i++) {
		if (triangles_[i].boneId > -1) triangles_[i].pBone = bones_[triangles_[i].boneId]; else triangles_[i].pBone = NULL;
	}*/
}

void Model::loadSma(string fileName) {
	vector<string> text;
	ifstream file;
	file.open(fileName.c_str());
	if (file.is_open()) {
		while (!file.eof()) {
			string tempStr;
			getline(file, tempStr);
			if (leftStr(tempStr, 2) != "//") {
				if (rightStr(tempStr, 1) == "\n") leftStr(&tempStr, tempStr.size()-1);
				text.push_back(tempStr);
			}
		}
		file.close();
	} else {
		cout << "File " << fileName << " could not be loaded" << endl;
		return;
	}
	if (text.back() == "") text.pop_back();

	unsigned boneCount = atoi(text.front().c_str()), line = 1;
	for (unsigned i = 0; i < boneCount; i++) {
		bone::animation newAnimation;
		unsigned boneId = atoi(text[line].c_str());
		line++;

		newAnimation.name = text[line];
		line++;
		newAnimation.length = atoi(text[line].c_str());
		line++;

		unsigned frameCount = atoi(text[line].c_str());
		line++;
		for (unsigned j = 0; j < frameCount; j++) {
			bone::keyFrame newFrame;
			newFrame.xRot = strtof(text[line].c_str(), NULL);
			line++;
			newFrame.yRot = strtof(text[line].c_str(), NULL);
			line++;
			newFrame.zRot = strtof(text[line].c_str(), NULL);
			line++;
			newFrame.step = atoi(text[line].c_str());
			line++;
			newAnimation.frames.push_back(newFrame);
		}
		bones_[boneId]->animations.push_back(newAnimation);
	}

	/*vector<string> text;
	ifstream file;
	file.open(fileName.c_str());
	if (file.is_open()) {
		while (!file.eof()) {
			string tempStr;
			getline(file, tempStr);
			if (rightStr(tempStr, 1) == "\n") leftStr(&tempStr, tempStr.size()-1);
			text.push_back(tempStr);
		}
		file.close();
	} else return;
	if (text.back() == "") text.pop_back();
	animation newAnimation;
	newAnimation.name = text[0];
	newAnimation.frames.clear();
	unsigned totalFrames = atoi(text[1].c_str());
	unsigned line = 2;
	for (unsigned i = 0; i < totalFrames; i++) {
		keyFrame newFrame;
		newFrame.step = atoi(text[line].c_str());
		line++;
		unsigned totalBoneData = atoi(text[line].c_str());
		line++;
		for (unsigned j = 0; j < totalBoneData; j++) {
			keyFrame::keyFrameData newBoneData;
			newBoneData.boneId = atoi(text[line].c_str());
			line++;
			newBoneData.xRot = strtof(text[line].c_str(), NULL);
			line++;
			newBoneData.yRot = strtof(text[line].c_str(), NULL);
			line++;
			newBoneData.zRot = strtof(text[line].c_str(), NULL);
			line++;
			newFrame.boneData.push_back(newBoneData);
		}
		newAnimation.frames.push_back(newFrame);
	}
	animations.push_back(newAnimation);*/
}

void Model::loadSmo(string path, string fileName, bufferUsageEnum bufferUsage) {//, void (*customBufferFunction)(GLuint*, Model*, void*), void * customData) {
	vector<string> text;
	ifstream file;
	file.open((path+fileName).c_str());
	if (file.is_open()) {
		while (!file.eof()) {
			string tempStr;
			getline(file, tempStr);
			if (leftStr(tempStr, 2) != "//") {
				if (rightStr(tempStr, 1) == "\n") leftStr(&tempStr, tempStr.size()-1);
				text.push_back(tempStr);
			}
		}
		file.close();
	} else {
		cout << "File " << fileName << " could not be loaded" << endl;
		return;
	}
	if (text.back() == "") text.pop_back();
	loadSms(path+text[1]);
	loadSmm(path, text[0], bufferUsage);
	for (unsigned i = 2; i < text.size(); i++) loadSma(path+text[i]);

	//glGenVertexArrays(1, &vao);
	//glBindVertexArray(vao);
	//if (customBufferFunction == NULL) initBufferSmo(); else (*customBufferFunction)(&vbo, this, customData);
	//glBindVertexArray(0);
	//for (char i = 0; i < 16; i++) glDisableVertexAttribArray(i);
}

void Model::initBufferObj(bufferUsageEnum bufferUsage) {
	GLfloat vertexArray[triangles_.size()*3*24];
	unsigned count = 0;
	for (unsigned i = 0; i < triangles_.size(); i++) {
		for (short j = 0; j < 3; j++) {
			vertexArray[count] = triangles_[i].coords[j].x;
			count++;
			vertexArray[count] = triangles_[i].coords[j].y;
			count++;
			vertexArray[count] = triangles_[i].coords[j].z;
			count++;
			vertexArray[count] = 1.0f;
			count++;
			vertexArray[count] = triangles_[i].surfaceNormal().x;//coords[j].normal_.x;
			count++;
			vertexArray[count] = triangles_[i].surfaceNormal().y;//triangles_[i].coords[j].normal_.y;
			count++;
			vertexArray[count] = triangles_[i].surfaceNormal().z;//triangles_[i].coords[j].normal_.z;
			count++;
			vertexArray[count] = materials_[triangles_[i].mtlNum].ambientColor.r;
			count++;
			vertexArray[count] = materials_[triangles_[i].mtlNum].ambientColor.g;
			count++;
			vertexArray[count] = materials_[triangles_[i].mtlNum].ambientColor.b;
			count++;
			vertexArray[count] = materials_[triangles_[i].mtlNum].diffuseColor.r;
			count++;
			vertexArray[count] = materials_[triangles_[i].mtlNum].diffuseColor.g;
			count++;
			vertexArray[count] = materials_[triangles_[i].mtlNum].diffuseColor.b;
			count++;
			vertexArray[count] = materials_[triangles_[i].mtlNum].specularColor.r;
			count++;
			vertexArray[count] = materials_[triangles_[i].mtlNum].specularColor.g;
			count++;
			vertexArray[count] = materials_[triangles_[i].mtlNum].specularColor.b;
			count++;
			vertexArray[count] = triangles_[i].texCoords[j].x;
			count++;
			vertexArray[count] = triangles_[i].texCoords[j].y;
			count++;
			vertexArray[count] = triangles_[i].texCoords[j].z;
			count++;
			vertexArray[count] = triangles_[i].mtlNum;
			count++;
			vertexArray[count] = materials_[triangles_[i].mtlNum].hasTexture;
			count++;
			vertexArray[count] = materials_[triangles_[i].mtlNum].shininess;
			count++;
			vertexArray[count] = materials_[triangles_[i].mtlNum].alpha;
			count++;
			vertexArray[count] = -1;
			count++;
		}
	}

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexArray), vertexArray, bufferUsage);
	glVertexAttribPointer(VERTEX_ATTRIBUTE, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*24, 0);
	glVertexAttribPointer(NORMAL_ATTRIBUTE, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*24, (const GLvoid*)(sizeof(GLfloat)*4));
	glVertexAttribPointer(COLOR0_ATTRIBUTE, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*24, (const GLvoid*)(sizeof(GLfloat)*7));
	glVertexAttribPointer(COLOR1_ATTRIBUTE, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*24, (const GLvoid*)(sizeof(GLfloat)*10));
	glVertexAttribPointer(COLOR2_ATTRIBUTE, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*24, (const GLvoid*)(sizeof(GLfloat)*13));
	glVertexAttribPointer(TEXTURE0_ATTRIBUTE, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*24, (const GLvoid*)(sizeof(GLfloat)*16));
	glVertexAttribPointer(EXTRA0_ATTRIBUTE, 1, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*24, (const GLvoid*)(sizeof(GLfloat)*19));
	glVertexAttribPointer(EXTRA1_ATTRIBUTE, 1, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*24, (const GLvoid*)(sizeof(GLfloat)*20));
	glVertexAttribPointer(EXTRA2_ATTRIBUTE, 1, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*24, (const GLvoid*)(sizeof(GLfloat)*21));
	glVertexAttribPointer(EXTRA3_ATTRIBUTE, 1, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*24, (const GLvoid*)(sizeof(GLfloat)*22));
	glVertexAttribPointer(EXTRA4_ATTRIBUTE, 1, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*24, (const GLvoid*)(sizeof(GLfloat)*23));

	glEnableVertexAttribArray(VERTEX_ATTRIBUTE);
	glEnableVertexAttribArray(NORMAL_ATTRIBUTE);
	glEnableVertexAttribArray(COLOR0_ATTRIBUTE);
	glEnableVertexAttribArray(COLOR1_ATTRIBUTE);
	glEnableVertexAttribArray(COLOR2_ATTRIBUTE);
	glEnableVertexAttribArray(TEXTURE0_ATTRIBUTE);
	glEnableVertexAttribArray(EXTRA0_ATTRIBUTE);
	glEnableVertexAttribArray(EXTRA1_ATTRIBUTE);
	glEnableVertexAttribArray(EXTRA2_ATTRIBUTE);
	glEnableVertexAttribArray(EXTRA3_ATTRIBUTE);
	glEnableVertexAttribArray(EXTRA4_ATTRIBUTE);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

/*void Model::initBufferSmo() {
	unsigned verticeCount = 0;
	for (unsigned i = 0; i < bones_.size(); i++) verticeCount += bones_[i]->triangles.size()*3;

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*verticeCount*23, NULL, GL_STATIC_DRAW);

	unsigned offset = 0;

	for (unsigned i = 0; i < bones_.size(); i++) {
		GLfloat vertexArray[bones_[i]->triangles.size()*3*23];
		bones_[i]->offset = offset;
		unsigned count = 0;
		for (unsigned j = 0; j < bones_[i]->triangles.size(); j++) {
			for (short k = 0; k < 3; k++) {
				vertexArray[count] = bones_[i]->triangles[j]->coords[k].x;
				count++;
				vertexArray[count] = bones_[i]->triangles[j]->coords[k].y;
				count++;
				vertexArray[count] = bones_[i]->triangles[j]->coords[k].z;
				count++;
				if (bones_[i]->triangles[j]->sharedCoord[k]) vertexArray[count] = 1.0f; else vertexArray[count] = 0.0f;
				count++;
				vertexArray[count] = bones_[i]->triangles[j]->coords[k].normal_.x;
				count++;
				vertexArray[count] = bones_[i]->triangles[j]->coords[k].normal_.y;
				count++;
				vertexArray[count] = bones_[i]->triangles[j]->coords[k].normal_.z;
				count++;
				vertexArray[count] = materials_[bones_[i]->triangles[j]->mtlNum].ambientColor.r;
				count++;
				vertexArray[count] = materials_[bones_[i]->triangles[j]->mtlNum].ambientColor.g;
				count++;
				vertexArray[count] = materials_[bones_[i]->triangles[j]->mtlNum].ambientColor.b;
				count++;
				vertexArray[count] = materials_[bones_[i]->triangles[j]->mtlNum].diffuseColor.r;
				count++;
				vertexArray[count] = materials_[bones_[i]->triangles[j]->mtlNum].diffuseColor.g;
				count++;
				vertexArray[count] = materials_[bones_[i]->triangles[j]->mtlNum].diffuseColor.b;
				count++;
				vertexArray[count] = materials_[bones_[i]->triangles[j]->mtlNum].specularColor.r;
				count++;
				vertexArray[count] = materials_[bones_[i]->triangles[j]->mtlNum].specularColor.g;
				count++;
				vertexArray[count] = materials_[bones_[i]->triangles[j]->mtlNum].specularColor.b;
				count++;
				vertexArray[count] = bones_[i]->triangles[j]->texCoords[k].x;
				count++;
				vertexArray[count] = bones_[i]->triangles[j]->texCoords[k].y;
				count++;
				vertexArray[count] = bones_[i]->triangles[j]->texCoords[k].z;
				count++;
				vertexArray[count] = bones_[i]->triangles[j]->mtlNum;
				count++;
				vertexArray[count] = materials_[bones_[i]->triangles[j]->mtlNum].hasTexture;
				count++;
				vertexArray[count] = materials_[bones_[i]->triangles[j]->mtlNum].shininess;
				count++;
				vertexArray[count] = materials_[bones_[i]->triangles[j]->mtlNum].alpha;
				count++;
			}
		}
		glBufferSubData(GL_ARRAY_BUFFER, sizeof(GLfloat)*offset*23, sizeof(vertexArray), vertexArray);
		offset += bones_[i]->triangles.size()*3;
	}

	glVertexAttribPointer(VERTEX_ATTRIBUTE, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*23, 0);
	glVertexAttribPointer(NORMAL_ATTRIBUTE, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*23, (const GLvoid*)(sizeof(GLfloat)*4));
	glVertexAttribPointer(COLOR0_ATTRIBUTE, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*23, (const GLvoid*)(sizeof(GLfloat)*7));
	glVertexAttribPointer(COLOR1_ATTRIBUTE, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*23, (const GLvoid*)(sizeof(GLfloat)*10));
	glVertexAttribPointer(COLOR2_ATTRIBUTE, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*23, (const GLvoid*)(sizeof(GLfloat)*13));
	glVertexAttribPointer(TEXTURE0_ATTRIBUTE, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*23, (const GLvoid*)(sizeof(GLfloat)*16));
	glVertexAttribPointer(EXTRA0_ATTRIBUTE, 1, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*23, (const GLvoid*)(sizeof(GLfloat)*19));
	glVertexAttribPointer(EXTRA1_ATTRIBUTE, 1, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*23, (const GLvoid*)(sizeof(GLfloat)*20));
	glVertexAttribPointer(EXTRA2_ATTRIBUTE, 1, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*23, (const GLvoid*)(sizeof(GLfloat)*21));
	glVertexAttribPointer(EXTRA3_ATTRIBUTE, 1, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*23, (const GLvoid*)(sizeof(GLfloat)*22));

	glEnableVertexAttribArray(VERTEX_ATTRIBUTE);
	glEnableVertexAttribArray(NORMAL_ATTRIBUTE);
	glEnableVertexAttribArray(COLOR0_ATTRIBUTE);
	glEnableVertexAttribArray(COLOR1_ATTRIBUTE);
	glEnableVertexAttribArray(COLOR2_ATTRIBUTE);
	glEnableVertexAttribArray(TEXTURE0_ATTRIBUTE);
	glEnableVertexAttribArray(EXTRA0_ATTRIBUTE);
	glEnableVertexAttribArray(EXTRA1_ATTRIBUTE);
	glEnableVertexAttribArray(EXTRA2_ATTRIBUTE);
	glEnableVertexAttribArray(EXTRA3_ATTRIBUTE);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Model::drawObj(Shader * shaderToUse) {
	shaderToUse->setUniform16(MODELVIEW_LOCATION, getMatrix(MODELVIEW_MATRIX));
	shaderToUse->setUniform16(PROJECTION_LOCATION, getMatrix(PROJECTION_MATRIX));

	glDrawArrays(GL_TRIANGLES, 0, triangles_.size()*3);
}*/

vec3 Model::calculatePoints(float nx, float ny, float nz, matrix4d matrix) {
	vec3 returnNormal;
	returnNormal.x = (nx*matrix[0])+(ny*matrix[4])+(nz*matrix[8])+matrix[12];
	returnNormal.y = (nx*matrix[1])+(ny*matrix[5])+(nz*matrix[9])+matrix[13];
	returnNormal.z = (nx*matrix[2])+(ny*matrix[6])+(nz*matrix[10])+matrix[14];
	return returnNormal;
}

void Model::calculateHitbox(bone * pBone, matrix4d matrix) {
	float nx, ny, nz;
	vec3 point[8];

	nx = 0.0f;
	ny = -pBone->hitbox.h/2.0f;
	nz = pBone->hitbox.w/2.0f;
	point[0] = calculatePoints(nx, ny, nz, matrix);

	nx = 0.0f;
	ny = pBone->hitbox.h/2.0f;
	nz = pBone->hitbox.w/2.0f;
	point[1] = calculatePoints(nx, ny, nz, matrix);

	nx = 0.0f;
	ny = pBone->hitbox.h/2.0f;
	nz = -pBone->hitbox.w/2.0f;
	point[2] = calculatePoints(nx, ny, nz, matrix);

	nx = 0.0f;
	ny = -pBone->hitbox.h/2.0f;
	nz = -pBone->hitbox.w/2.0f;
	point[3] = calculatePoints(nx, ny, nz, matrix);

	nx = pBone->hitbox.l;
	ny = -pBone->hitbox.h/2.0f;
	nz = pBone->hitbox.w/2.0f;
	point[4] = calculatePoints(nx, ny, nz, matrix);

	nx = pBone->hitbox.l;
	ny = pBone->hitbox.h/2.0f;
	nz = pBone->hitbox.w/2.0f;
	point[5] = calculatePoints(nx, ny, nz, matrix);

	nx = pBone->hitbox.l;
	ny = pBone->hitbox.h/2.0f;
	nz = -pBone->hitbox.w/2.0f;
	point[6] = calculatePoints(nx, ny, nz, matrix);

	nx = pBone->hitbox.l;
	ny = -pBone->hitbox.h/2.0f;
	nz = -pBone->hitbox.w/2.0f;
	point[7] = calculatePoints(nx, ny, nz, matrix);

	float lLowerBound = point[0].x, lUpperBound = point[0].x, wLowerBound = point[0].z, wUpperBound = point[0].z, hLowerBound = point[0].y,
			hUpperBound = point[0].y;
	for (short i = 0; i < 8; i++) {
		if (point[i].x < lLowerBound) lLowerBound = point[i].x; else if (point[i].x > lUpperBound) lUpperBound = point[i].x;
		if (point[i].z < wLowerBound) wLowerBound = point[i].z; else if (point[i].z > wUpperBound)	wUpperBound = point[i].z;
		if (point[i].y < hLowerBound) hLowerBound = point[i].y; else if (point[i].y > hUpperBound) hUpperBound = point[i].y;
	}
	pBone->hitbox.rl = lUpperBound-lLowerBound;
	pBone->hitbox.rw = wUpperBound-wLowerBound;
	pBone->hitbox.rh = hUpperBound-hLowerBound;
}

/*void Model::drawBone(bone * pBone, Shader * shaderToUse, bool skipHitboxes) {
	pushMatrix();
		matrix4d modelMatrix, modelNormMatrix;
		pushMatrix();
			copyMatrix(IDENTITY_MATRIX, MODELVIEW_MATRIX);
			if (!skipHitboxes) {
				pushMatrix();
					rotateMatrix(pBone->xRot, 1.0f, 0.0f, 0.0f);
					rotateMatrix(pBone->yRot, 0.0f, 1.0f, 0.0f);
					rotateMatrix(pBone->zRot, 0.0f, 0.0f, 1.0f);
					modelNormMatrix = getMatrix(MODELVIEW_MATRIX);
				popMatrix();
			}
			translateMatrix(pBone->x, pBone->y, pBone->z);
			rotateMatrix(pBone->xRot, 1.0f, 0.0f, 0.0f);
			rotateMatrix(pBone->yRot, 0.0f, 1.0f, 0.0f);
			rotateMatrix(pBone->zRot, 0.0f, 0.0f, 1.0f);
			translateMatrix(-pBone->x, -pBone->y, -pBone->z);
			modelMatrix = getMatrix(MODELVIEW_MATRIX);
		popMatrix();

		shaderToUse->setUniform16(EXTRA0_LOCATION, getMatrix(MODELVIEW_MATRIX));

		copyMatrix(getMatrix(MODELVIEW_MATRIX)*modelMatrix, MODELVIEW_MATRIX);

		shaderToUse->setUniform16(MODELVIEW_LOCATION, getMatrix(MODELVIEW_MATRIX));
		shaderToUse->setUniform16(PROJECTION_LOCATION, getMatrix(PROJECTION_MATRIX));

		//glDrawArrays(GL_TRIANGLES, pBone->offset, pBone->triangles.size()*3);

		if (!skipHitboxes) {
			if (pBone->parent == NULL) {
				pBone->hitbox.x = 0.0f;
				pBone->hitbox.y = 0.0f;
				pBone->hitbox.z = 0.0f;
			} else {
				pBone->hitbox.x = pBone->parent->hitbox.x+pBone->parent->endX;
				pBone->hitbox.y = pBone->parent->hitbox.y+pBone->parent->endY;
				pBone->hitbox.z = pBone->parent->hitbox.z+pBone->parent->endZ;
			}
			float nx = pBone->endX;
			float ny = pBone->endY;
			float nz = pBone->endZ;
			pBone->endX = (nx*modelNormMatrix[0])+(ny*modelNormMatrix[4])+(nz*modelNormMatrix[8])+modelNormMatrix[12];
			pBone->endY = (nx*modelNormMatrix[1])+(ny*modelNormMatrix[5])+(nz*modelNormMatrix[9])+modelNormMatrix[13];
			pBone->endZ = (nx*modelNormMatrix[2])+(ny*modelNormMatrix[6])+(nz*modelNormMatrix[10])+modelNormMatrix[14];
			calculateHitbox(pBone, modelNormMatrix);
		}

		if (pBone->child.size() > 0) {
			for (unsigned i = 0; i < pBone->child.size(); i++) drawBone(pBone->child[i], shaderToUse, skipHitboxes);
		}
	popMatrix();
}*/

void Model::getBoneModelviewMatrices(matrix4d * matrixArray, bone * pBone) {
	pushMatrix();
		translateMatrix(pBone->x, pBone->y, pBone->z);
		rotateMatrix(pBone->xRot, 1.0f, 0.0f, 0.0f);
		rotateMatrix(pBone->yRot, 0.0f, 1.0f, 0.0f);
		rotateMatrix(pBone->zRot, 0.0f, 0.0f, 1.0f);
		translateMatrix(-pBone->x, -pBone->y, -pBone->z);
		matrixArray[pBone->id] = getMatrix(MODELVIEW_MATRIX);

		for (unsigned i = 0; i < pBone->child.size(); i++) getBoneModelviewMatrices(matrixArray, pBone->child[i]);
	popMatrix();
}

void Model::setBoneRotationsFromAnimation(unsigned animationId, float frame, bone * pBone) {
	int index = pBone->animations[animationId].frameIndex(frame);
	if (index == -1) {
		if (frame > pBone->animations[animationId].frames.back().step) {
			pBone->xRot = pBone->animations[animationId].frames.back().xRot;
			pBone->yRot = pBone->animations[animationId].frames.back().yRot;
			pBone->zRot = pBone->animations[animationId].frames.back().zRot;
		} else {
			bone::keyFrame * previousFrame, * nextFrame;
			int i = 0;
			while (frame > pBone->animations[animationId].frames[i].step) i++;
			previousFrame = &pBone->animations[animationId].frames[i-1];
			nextFrame = &pBone->animations[animationId].frames[i];

			float xDiff, yDiff, zDiff, xDiff1 = nextFrame->xRot-previousFrame->xRot, yDiff1 = nextFrame->yRot-previousFrame->yRot,
					zDiff1 = nextFrame->zRot-previousFrame->zRot, xDiff2 = (360.0f-abs(previousFrame->xRot))-abs(nextFrame->xRot),
					yDiff2 = (360.0f-abs(previousFrame->yRot))-abs(nextFrame->yRot), zDiff2 = (360.0f-abs(previousFrame->zRot))-abs(nextFrame->zRot),
					stepDiff = nextFrame->step-previousFrame->step;

			xDiff = (abs(xDiff1) < xDiff2) ? xDiff1 : ((previousFrame->xRot < 0.0f) ? -xDiff2 : xDiff2);
			yDiff = (abs(yDiff1) < yDiff2) ? yDiff1 : ((previousFrame->yRot < 0.0f) ? -yDiff2 : yDiff2);
			zDiff = (abs(zDiff1) < zDiff2) ? zDiff1 : ((previousFrame->zRot < 0.0f) ? -zDiff2 : zDiff2);

			float multiplier = (frame-previousFrame->step)/stepDiff;
			pBone->xRot = previousFrame->xRot+(xDiff*multiplier);
			pBone->yRot = previousFrame->yRot+(yDiff*multiplier);
			pBone->zRot = previousFrame->zRot+(zDiff*multiplier);
		}
	} else {
		pBone->xRot = pBone->animations[animationId].frames[index].xRot;
		pBone->yRot = pBone->animations[animationId].frames[index].yRot;
		pBone->zRot = pBone->animations[animationId].frames[index].zRot;
	}

	for (unsigned i = 0; i < pBone->child.size(); i++) setBoneRotationsFromAnimation(animationId, frame, pBone->child[i]);
}

string Model::name() {
	return name_;
}

void Model::draw(float x, float y, float z, float xRotation, float yRotation, float zRotation, float xScale, float yScale, float zScale, float frame,
		int currentAnimationId, bool skipAnimation, bool skipHitboxes) {
	Shader * shaderToUse;
	if (boundShader_ != NULL) shaderToUse = boundShader_; else shaderToUse = ::boundShader();

	if (shaderToUse != NULL) {
		glUseProgram(shaderToUse->program_);
		glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
		shaderToUse->setUniform1(TEXSAMPLER_LOCATION, 0);

		setMatrix(MODELVIEW_MATRIX);
		pushMatrix();
			translateMatrix(x, y, z);
			rotateMatrix(xRotation, 1.0f, 0.0f, 0.0f);
			rotateMatrix(yRotation, 0.0f, 1.0f, 0.0f);
			rotateMatrix(zRotation, 0.0f, 0.0f, 1.0f);
			scaleMatrix(xScale, yScale, zScale);

			shaderToUse->setUniform16(MODELVIEW_LOCATION, getMatrix(MODELVIEW_MATRIX));
			shaderToUse->setUniform16(PROJECTION_LOCATION, getMatrix(PROJECTION_MATRIX));

			if (!skipAnimation && (bones_.size() > 0)) {
				setBoneRotationsFromAnimation(currentAnimationId, frame, bones_.front());
				matrix4d matrixArray[bones_.size()];
				getBoneModelviewMatrices(matrixArray, bones_.front());
				shaderToUse->setUniform16(EXTRA0_LOCATION, (float*)matrixArray, bones_.size());
			}

			glBindVertexArray(vao);
			glDrawArrays(GL_TRIANGLES, 0, vertexCount);
			glBindVertexArray(0);
			/*glBindVertexArray(vao);
			if (loadedFromObj) drawObj(shaderToUse); else {
				if (!skipAnimation) {
					if (animations.size() > 0) {
						if (animations[currentAnimationId].frames.size() > 1) {
							keyFrame * nextKeyFrame, * thisKeyFrame;

							unsigned currentKeyFrame = 0;
							while (frame > animations[currentAnimationId].frames.back().step)
								frame -= animations[currentAnimationId].frames.back().step;
							while (frame > (int)animations[currentAnimationId].frames[currentKeyFrame+1].step) {
								currentKeyFrame++;
								if (currentKeyFrame >= animations[currentAnimationId].frames.size()) {
									currentKeyFrame = 0;
									frame -= animations[currentAnimationId].frames.back().step;
									break;
								}
							}

							thisKeyFrame = &animations[currentAnimationId].frames[currentKeyFrame];
							if (currentKeyFrame+1 < animations[currentAnimationId].frames.size())
								nextKeyFrame = &animations[currentAnimationId].frames[currentKeyFrame+1]; else
									nextKeyFrame = &animations[currentAnimationId].frames.front();

							float tempFrame = frame-(float)thisKeyFrame->step;
							int stepDiff = nextKeyFrame->step-thisKeyFrame->step;

							for (unsigned i = 0; i < bones_.size(); i++) {
								float diff, diff1 = nextKeyFrame->boneData[i].xRot-thisKeyFrame->boneData[i].xRot, diff2 = 360-nextKeyFrame->boneData[i].xRot;
								if (abs(diff1) < abs(diff2)) diff = diff1; else diff = diff2;
								bones_[thisKeyFrame->boneData[i].boneId]->xRot = thisKeyFrame->boneData[i].xRot+((diff/(float)stepDiff)*(float)tempFrame);

								diff1 = nextKeyFrame->boneData[i].yRot-thisKeyFrame->boneData[i].yRot, diff2 = 360-nextKeyFrame->boneData[i].yRot;
								if (abs(diff1) < abs(diff2)) diff = diff1; else diff = diff2;
								bones_[thisKeyFrame->boneData[i].boneId]->yRot = thisKeyFrame->boneData[i].yRot+((diff/(float)stepDiff)*(float)tempFrame);

								diff1 = nextKeyFrame->boneData[i].zRot-thisKeyFrame->boneData[i].zRot, diff2 = 360-nextKeyFrame->boneData[i].zRot;
								if (abs(diff1) < abs(diff2)) diff = diff1; else diff = diff2;
								bones_[thisKeyFrame->boneData[i].boneId]->zRot = thisKeyFrame->boneData[i].zRot+((diff/(float)stepDiff)*(float)tempFrame);
							}
						}
					}
				}
				drawBone(bones_[0], shaderToUse, skipHitboxes);
				glBindVertexArray(0);
			}*/
		popMatrix();
	}
	if (::boundShader() != NULL) glUseProgram(::boundShader()->program_); else glUseProgram(0);
}

void Model::draw(Object * object, bool skipAnimation, bool skipHitboxes) {
	Shader * shaderToUse;
	if (object->boundShader_ != NULL) shaderToUse = object->boundShader_; else if (boundShader_ != NULL) shaderToUse = boundShader_; else shaderToUse = ::boundShader();

	if (shaderToUse != NULL) {
		glUseProgram(shaderToUse->program_);
		glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
		shaderToUse->setUniform1(TEXSAMPLER_LOCATION, 0);

		setMatrix(MODELVIEW_MATRIX);
		pushMatrix();
			translateMatrix(object->x_, object->y_, object->z_);
			rotateMatrix(object->xRotation_, 1.0f, 0.0f, 0.0f);
			rotateMatrix(object->yRotation_, 0.0f, 1.0f, 0.0f);
			rotateMatrix(object->zRotation_, 0.0f, 0.0f, 1.0f);
			scaleMatrix(object->xScale_, object->yScale_, object->zScale_);

			shaderToUse->setUniform16(MODELVIEW_LOCATION, getMatrix(MODELVIEW_MATRIX));
			shaderToUse->setUniform16(PROJECTION_LOCATION, getMatrix(PROJECTION_MATRIX));

			if (!skipAnimation && (bones_.size() > 0)) {
				setBoneRotationsFromAnimation(object->currentAnimationId, object->frame_, bones_.front());
				matrix4d matrixArray[bones_.size()];
				getBoneModelviewMatrices(matrixArray, bones_.front());
				shaderToUse->setUniform16(EXTRA0_LOCATION, (float*)matrixArray, bones_.size());
			}

			glBindVertexArray(vao);
			glDrawArrays(GL_TRIANGLES, 0, vertexCount);
			glBindVertexArray(0);
		popMatrix();
	}
	if (::boundShader() != NULL) glUseProgram(::boundShader()->program_); else glUseProgram(0);
}

void Model::bindShader(Shader * shader) {
	boundShader_ = shader;
}

Shader * Model::boundShader() {
	return boundShader_;
}

int Model::boneId(string boneName) {
	for (unsigned i = 0; i < bones_.size(); i++) if (bones_[i]->name == boneName) return i;
	return -1;
}

string Model::boneName(unsigned boneId) {
	if (boneId >= bones_.size()) return "";
	return bones_[boneId]->name;
}

int Model::animationId(string searchName) {
	if (bones_.size() > 0) {
		for (unsigned i = 0; i < bones_.front()->animations.size(); i++) if (bones_.front()->animations[i].name == searchName) return i;
	}
	return -1;
}

void Model::setFramerate(unsigned newFramerate) {
	framerate_ = newFramerate;
}

unsigned Model::framerate() {
	return framerate_;
}

vector<bone *> * Model::bones() {
	return &bones_;
}

vector<Model::triangle> * Model::triangles() {
	return &triangles_;
}

vector<Model::material> * Model::materials() {
	return &materials_;
}

GLuint * Model::vboPointer() {
	return &vbo;
}

int bone::animation::frameIndex(unsigned step) {
	for (unsigned i = 0; i < frames.size(); i++) {
		if (frames[i].step == step) {
			return i;
		}
	}
	return -1;
}

Model * model(string searchName) {
	int letter = numCharInAlphabet(searchName[0]);
	Model * returnModel = NULL;
	if (allModels[letter].size() > 0) {
		for (unsigned i = 0; i < allModels[letter].size(); i++) {
			if (allModels[letter][i]->name() == searchName) returnModel = allModels[letter][i];
		}
	}
	return returnModel;
}

Model * addModel(string newName, string path, string fileName, unsigned framerate, bufferUsageEnum bufferUsage, void (*customBufferFunction)(GLuint*, Model*, void*),
		void * customData) {
	int letter = numCharInAlphabet(newName[0]);
	Model * newModel = new Model(newName, path, fileName, framerate, bufferUsage, customBufferFunction, customData);
	allModels[letter].push_back(newModel);
	return newModel;
}

void destroyModel(string searchName) {
	int letter = numCharInAlphabet(searchName[0]);
	if (allModels[letter].size() > 0) {
		for (unsigned i = 0; i < allModels[letter].size(); i++) {
			if (allModels[letter][i]->name() == searchName) {
				delete allModels[letter][i];
				allModels[letter].erase(allModels[letter].begin()+i);
				break;
			}
		}
	}
}

void destroyAllModels() {
	for (int i = 0; i < 27; i++) {
		if (allModels[i].size() > 0) {
			for (unsigned j = 0; j < allModels[i].size(); j++) delete allModels[i][j];
			allModels[i].clear();
		}
	}
}

}
