//============================================================================
// Name        : Model.cpp
// Author      : Max Foster
// Created on  : 30 May 2011
// Version     : 1.0
// Copyright   : http://creativecommons.org/licenses/by/3.0/
// Description : SuperMaximo GameLibrary 3D Model class
//============================================================================

#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
using namespace std;

#include <GL/glew.h>
#include <SDL/SDL_image.h>

#include "../../headers/classes/Model.h"
#include "../../headers/classes/Object.h"
#include "../../headers/classes/Shader.h"
#include "../../headers/Display.h"
#include "../../headers/Utils.h"
using namespace SuperMaximo;

struct vertexNormalAssoication {
	vec3 vertex;
	vec3 normal;
	vector<void*> verticesToUpdate;
	vector<vec3> surfaceNormals;
};

namespace SuperMaximo {

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
	if (len == 0.0f) len = 1.0f;
	normal_ /= len;

	return normal_;
}

Model::Model(const string & newName, const string & path, const string & fileName, unsigned framerate,
		bufferUsageEnum bufferUsage, void (*customBufferFunction)(GLuint*, Model*, void*), void * customData) {
	name_ = newName;
	boundShader_ = NULL;
	framerate_ = framerate;
	if (lowerCase(leftStr(rightStr(fileName, 3), 2)) == "sm") {
		switch (rightStr(fileName, 1)[0]) {
		case 'o': loadSmo(path, fileName, bufferUsage); break;
		case 'm': loadSmm(path, fileName, bufferUsage); break;
		}
	} else if (lowerCase(rightStr(fileName, 3)) == "obj")
		loadObj(path, fileName, bufferUsage, customBufferFunction, customData);
}

Model::~Model() {
	glDeleteTextures(1, &texture);
	for (unsigned i = 0; i < bones_.size(); i++) delete bones_[i];
	glDeleteBuffers(1, &vbo);
	if (vertexArrayObjectSupported()) glDeleteVertexArrays(1, &vao);
}

void Model::loadObj(const string & path, const string & fileName, bufferUsageEnum bufferUsage,
		void (*customBufferFunction)(GLuint*, Model*, void*), void * customData) {
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

	bool blocky = false;
	for (unsigned i = 0; i < objText.size(); i++) {
		while ((leftStr(objText[i], 1) == " ") || (leftStr(objText[i], 1) == "\t"))
			rightStr(&objText[i], objText[i].size()-1);
		while ((rightStr(objText[i], 1) == " ") || (rightStr(objText[i], 1) == "\t")
				|| (*(rightStr(objText[i], 1).c_str()) == 13)) leftStr(&objText[i], objText[i].size()-1);
		if (leftStr(objText[i], 7) == "#BLOCKY") blocky = true;
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
		while ((leftStr(mtlText[i], 1) == " ") || (leftStr(mtlText[i], 1) == "\t"))
			rightStr(&mtlText[i], mtlText[i].size()-1);
		while ((rightStr(mtlText[i], 1) == " ") || (rightStr(mtlText[i], 1) == "\t")
				|| (*(rightStr(mtlText[i], 1).c_str()) == 13)) leftStr(&mtlText[i], mtlText[i].size()-1);
	}
	bool initialised = false;
	unsigned totalMaterials = 0;
	for (unsigned i = 0; i < mtlText.size(); i++) {
		if (lowerCase(leftStr(mtlText[i], 6)) == "newmtl") totalMaterials++;
	}
	textureCount = totalMaterials;
	for (unsigned i = 0; i < mtlText.size(); i++) {
		if (lowerCase(leftStr(mtlText[i], 6)) == "newmtl") {
			material newMaterial;
			newMaterial.name = rightStr(mtlText[i], mtlText[i].size()-7);
			newMaterial.hasTexture = false;

			string kStr = "";
			for (unsigned j = i+1; j < mtlText.size(); j++) {
				if (lowerCase(leftStr(mtlText[j], 2)) == "ka") {
					kStr = rightStr(mtlText[j], mtlText[j].size()-3);
					break;
				}
			}
			if (kStr == "") kStr = "0.0000 0.0000 0.0000";
			int j = kStr.find(" ");
			newMaterial.ambientColor.r = strtod(leftStr(kStr, j).c_str(), NULL);
			rightStr(&kStr, kStr.size()-(j+1));

			j = kStr.find(" ");
			newMaterial.ambientColor.g = strtod(leftStr(kStr, j).c_str(), NULL);
			rightStr(&kStr, kStr.size()-(j+1));

			newMaterial.ambientColor.b = strtod(kStr.c_str(), NULL);


			kStr = "";
			for (unsigned j = i+1; j < mtlText.size(); j++) {
				if (lowerCase(leftStr(mtlText[j], 2)) == "kd") {
					kStr = rightStr(mtlText[j], mtlText[j].size()-3);
					break;
				}
			}
			if (kStr == "") kStr = "0.0000 0.0000 0.0000";
			j = kStr.find(" ");
			newMaterial.diffuseColor.r = strtod(leftStr(kStr, j).c_str(), NULL);
			rightStr(&kStr, kStr.size()-(j+1));

			j = kStr.find(" ");
			newMaterial.diffuseColor.g = strtod(leftStr(kStr, j).c_str(), NULL);
			rightStr(&kStr, kStr.size()-(j+1));

			newMaterial.diffuseColor.b = strtod(kStr.c_str(), NULL);


			kStr = "";
			for (unsigned j = i+1; j < mtlText.size(); j++) {
				if (lowerCase(leftStr(mtlText[j], 2)) == "ks") {
					kStr = rightStr(mtlText[j], mtlText[j].size()-3);
					break;
				}
			}
			if (kStr == "") kStr = "0.0000 0.0000 0.0000";
			j = kStr.find(" ");
			newMaterial.specularColor.r = strtod(leftStr(kStr, j).c_str(), NULL);
			rightStr(&kStr, kStr.size()-(j+1));

			j = kStr.find(" ");
			newMaterial.specularColor.g = strtod(leftStr(kStr, j).c_str(), NULL);
			rightStr(&kStr, kStr.size()-(j+1));

			newMaterial.specularColor.b = strtod(kStr.c_str(), NULL);


			kStr = "";
			for (unsigned j = i+1; j < mtlText.size(); j++) {
				if (lowerCase(leftStr(mtlText[j], 1)) == "d") {
					kStr = rightStr(mtlText[j], mtlText[j].size()-2);
					break;
				}
			}
			if (kStr == "") kStr = "1.0000";
			newMaterial.alpha = strtod(kStr.c_str(), NULL);

			kStr = "";
			for (unsigned j = i+1; j < mtlText.size(); j++) {
				if (lowerCase(leftStr(mtlText[j], 2)) == "ns") {
					kStr = rightStr(mtlText[j], mtlText[j].size()-3);
					break;
				}
			}
			if (kStr == "") kStr = "0.0000";
			newMaterial.shininess = strtod(kStr.c_str(), NULL);

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
						if (texture2dArrayDisabled()) {
							glBindTexture(GL_TEXTURE_2D, texture);
							glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
							glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
							if (openGlVersion() >= 3.0f) glGenerateMipmap(GL_TEXTURE_2D);
							else glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);

							glTexImage2D(GL_TEXTURE_2D, 0, image->format->BytesPerPixel, image->w*totalMaterials,
									image->h, 0, textureFormat, GL_UNSIGNED_BYTE, NULL);
						} else {
							glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
							glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
							glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
							if (openGlVersion() >= 3.0f) glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
							else glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_GENERATE_MIPMAP, GL_TRUE);

							glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, image->format->BytesPerPixel, image->w, image->h,
									totalMaterials, 0, textureFormat, GL_UNSIGNED_BYTE, NULL);
						}

					}
					if (texture2dArrayDisabled())
						glTexSubImage2D(GL_TEXTURE_2D, 0, image->w*i, 0, image->w, image->h, textureFormat,
								GL_UNSIGNED_BYTE, image->pixels);
					else glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, materials_.size(), image->w, image->h, 1,
							textureFormat, GL_UNSIGNED_BYTE, image->pixels);
					SDL_FreeSurface(image);
					newMaterial.hasTexture = true;
				}
			}
			materials_.push_back(newMaterial);
		}
	}

	int mtlNum = 0;
	for (unsigned i = 0; i < objText.size(); i++) {
		if (lowerCase(leftStr(objText[i], 1)) == "v") {
			if (lowerCase(leftStr(objText[i], 2)) == "vt") {
				vertex coord;
				string vertStr = rightStr(objText[i], objText[i].size()-3);
				while ((leftStr(vertStr, 1) == " ") || (leftStr(vertStr, 1)) == "\t")
					rightStr(&vertStr, vertStr.size()-1);

				int j = vertStr.find(" ");
				coord.x  = strtod(leftStr(vertStr, j).c_str(), NULL);
				rightStr(&vertStr, vertStr.size()-(j+1));
				while ((leftStr(vertStr, 1) == " ") || (leftStr(vertStr, 1)) == "\t")
					rightStr(&vertStr, vertStr.size()-1);

				j = vertStr.find(" ");
				if (j == -1) coord.y = strtod(vertStr.c_str(), NULL);
				else coord.y = strtod(leftStr(vertStr, j).c_str(), NULL);
				texCoord.push_back(coord);
			} else if (lowerCase(leftStr(objText[i], 2)) == "v ") {
				vertex coord;
				string vertStr = rightStr(objText[i], objText[i].size()-2);
				while ((leftStr(vertStr, 1) == " ") || (leftStr(vertStr, 1)) == "\t")
					rightStr(&vertStr, vertStr.size()-1);

				int j = vertStr.find(" ");
				coord.x = strtod(leftStr(vertStr, j).c_str(), NULL);
				rightStr(&vertStr, vertStr.size()-(j+1));
				while ((leftStr(vertStr, 1) == " ") || (leftStr(vertStr, 1)) == "\t")
					rightStr(&vertStr, vertStr.size()-1);

				j = vertStr.find(" ");
				coord.y = strtod(leftStr(vertStr, j).c_str(), NULL);
				rightStr(&vertStr, vertStr.size()-(j+1));
				while ((leftStr(vertStr, 1) == " ") || (leftStr(vertStr, 1)) == "\t")
					rightStr(&vertStr, vertStr.size()-1);

				coord.z = strtod(vertStr.c_str(), NULL);
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

	if (blocky) {
		for (unsigned i = 0; i < triangles_.size(); i++) {
			for (short j = 0; j < 3; j++) triangles_[i].coords[j].normal_ = triangles_[i].surfaceNormal();
		}
	} else {
		vector<vertexNormalAssoication> vertexNormalAssociations;

		vertexNormalAssociations.reserve(vertices.size());
		for (unsigned i = 0; i < vertices.size(); i++) {
			vertexNormalAssociations.push_back(vertexNormalAssoication());
			vertexNormalAssociations.back().vertex = vec3(vertices[i].x, vertices[i].y, vertices[i].z);
		}

		for (unsigned i = 0; i < triangles_.size(); i++) {
			for (short j = 0; j < 3; j++) {
				for (unsigned k = 0; k < vertexNormalAssociations.size(); k++) {
					if ((vertexNormalAssociations[k].vertex.x == triangles_[i].coords[j].x)
							&& (vertexNormalAssociations[k].vertex.y == triangles_[i].coords[j].y)
								&& (vertexNormalAssociations[k].vertex.z == triangles_[i].coords[j].z)) {
						vertexNormalAssociations[k].verticesToUpdate.push_back(&triangles_[i].coords[j]);
						vertexNormalAssociations[k].surfaceNormals.push_back(triangles_[i].surfaceNormal());
						break;
					}
				}
			}
		}

		for (unsigned i = 0; i < vertexNormalAssociations.size(); i++) {
			vec3 normalTotal;
			normalTotal.x = normalTotal.y = normalTotal.z = 0;
			for (unsigned j = 0; j < vertexNormalAssociations[i].surfaceNormals.size(); j++) {
				normalTotal.x += vertexNormalAssociations[i].surfaceNormals[j].x;
				normalTotal.y += vertexNormalAssociations[i].surfaceNormals[j].y;
				normalTotal.z += vertexNormalAssociations[i].surfaceNormals[j].z;
			}

			float len = sqrt((normalTotal.x*normalTotal.x)+(normalTotal.y*normalTotal.y)+(normalTotal.z*normalTotal.z));
			if (len == 0) len = 1;
			normalTotal.x /= len;
			normalTotal.y /= len;
			normalTotal.z /= len;

			vertexNormalAssociations[i].normal = normalTotal;

			for (unsigned j = 0; j < vertexNormalAssociations[i].verticesToUpdate.size(); j++)
				((vertex*)(vertexNormalAssociations[i].verticesToUpdate[j]))->normal_ = normalTotal;
		}
	}
	vertexCount_ = triangles_.size()*3;

	if (vertexArrayObjectSupported()) {
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
	}
	if (customBufferFunction == NULL) initBufferObj(bufferUsage); else (*customBufferFunction)(&vbo, this, customData);
	if (vertexArrayObjectSupported()) glBindVertexArray(0);
	for (char i = 0; i < 16; i++) glDisableVertexAttribArray(i);
}

void Model::loadSmm(const string & path, const string & fileName, bufferUsageEnum bufferUsage) {
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

	vertexCount_ = atoi(text.front().c_str());
	unsigned arraySize = vertexCount_*24;
	GLfloat * data = new (nothrow) GLfloat[arraySize];
	if (data == NULL) {
		cout << "Could not allocate memory for buffering the model" << endl;
		return;
	}
	for (unsigned i = 0; i < arraySize; i++) data[i] = strtod(text[i+1].c_str(), NULL);

	if (vertexArrayObjectSupported()) {
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
	}

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*vertexCount_*24, data, bufferUsage);

	glVertexAttribPointer(VERTEX_ATTRIBUTE, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*24, 0);
	glVertexAttribPointer(NORMAL_ATTRIBUTE, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*24,
			(const GLvoid*)(sizeof(GLfloat)*4));
	glVertexAttribPointer(COLOR0_ATTRIBUTE, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*24,
			(const GLvoid*)(sizeof(GLfloat)*7));
	glVertexAttribPointer(COLOR1_ATTRIBUTE, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*24,
			(const GLvoid*)(sizeof(GLfloat)*10));
	glVertexAttribPointer(COLOR2_ATTRIBUTE, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*24,
			(const GLvoid*)(sizeof(GLfloat)*13));
	glVertexAttribPointer(TEXTURE0_ATTRIBUTE, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*24,
			(const GLvoid*)(sizeof(GLfloat)*16));
	glVertexAttribPointer(EXTRA0_ATTRIBUTE, 1, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*24,
			(const GLvoid*)(sizeof(GLfloat)*19));
	glVertexAttribPointer(EXTRA1_ATTRIBUTE, 1, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*24,
			(const GLvoid*)(sizeof(GLfloat)*20));
	glVertexAttribPointer(EXTRA2_ATTRIBUTE, 1, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*24,
			(const GLvoid*)(sizeof(GLfloat)*21));
	glVertexAttribPointer(EXTRA3_ATTRIBUTE, 1, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*24,
			(const GLvoid*)(sizeof(GLfloat)*22));
	glVertexAttribPointer(EXTRA4_ATTRIBUTE, 1, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*24,
			(const GLvoid*)(sizeof(GLfloat)*23));

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

	if (vertexArrayObjectSupported()) glBindVertexArray(0);
	delete[] data;

	textureCount = atoi(text[arraySize+1].c_str());
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
				if (texture2dArrayDisabled()) {
					glBindTexture(GL_TEXTURE_2D, texture);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					if (openGlVersion() >= 3.0f) glGenerateMipmap(GL_TEXTURE_2D);
					else glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);

					glTexImage2D(GL_TEXTURE_2D, 0, image->format->BytesPerPixel, image->w*textureCount,
							image->h, 0, textureFormat, GL_UNSIGNED_BYTE, NULL);
				} else {
					glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
					glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
					glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					if (openGlVersion() >= 3.0f) glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
						else glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_GENERATE_MIPMAP, GL_TRUE);
					glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, image->format->BytesPerPixel, image->w, image->h,
							textureCount, 0, textureFormat, GL_UNSIGNED_BYTE, NULL);
				}
			}
			if (texture2dArrayDisabled())
				glTexSubImage2D(GL_TEXTURE_2D, 0, image->w*i, 0, image->w, image->h, textureFormat,
						GL_UNSIGNED_BYTE, image->pixels);
			else glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, image->w, image->h, 1, textureFormat,
					GL_UNSIGNED_BYTE, image->pixels);
			SDL_FreeSurface(image);
		}
		materials_.push_back(material());
		materials_.back().name = "";
		materials_.back().fileName = text[arraySize+2+i];
	}
}

void Model::loadSms(const string & fileName) {
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
		newBone->x = strtod(text[line].c_str(), NULL);
		line++;
		newBone->y = strtod(text[line].c_str(), NULL);
		line++;
		newBone->z = strtod(text[line].c_str(), NULL);
		line++;
		newBone->endX = strtod(text[line].c_str(), NULL);
		line++;
		newBone->endY = strtod(text[line].c_str(), NULL);
		line++;
		newBone->endZ = strtod(text[line].c_str(), NULL);
		line++;
		int boneParentId = atoi(text[line].c_str());
		if (boneParentId < 0) newBone->parent = NULL; else {
			newBone->parent = bones_[boneParentId];
			bones_[boneParentId]->child.push_back(newBone);
		}
		line++;
		newBone->rotationUpperLimit.x = strtod(text[line].c_str(), NULL);
		line++;
		newBone->rotationUpperLimit.y = strtod(text[line].c_str(), NULL);
		line++;
		newBone->rotationUpperLimit.z = strtod(text[line].c_str(), NULL);
		line++;
		newBone->rotationLowerLimit.x = strtod(text[line].c_str(), NULL);
		line++;
		newBone->rotationLowerLimit.y = strtod(text[line].c_str(), NULL);
		line++;
		newBone->rotationLowerLimit.z = strtod(text[line].c_str(), NULL);
		line++;

		bones_.push_back(newBone);
	}
}

void Model::loadSma(const string & fileName) {
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
			newFrame.xRot = strtod(text[line].c_str(), NULL);
			line++;
			newFrame.yRot = strtod(text[line].c_str(), NULL);
			line++;
			newFrame.zRot = strtod(text[line].c_str(), NULL);
			line++;
			newFrame.step = atoi(text[line].c_str());
			line++;
			newAnimation.frames.push_back(newFrame);
		}
		bones_[boneId]->animations.push_back(newAnimation);
	}
}

void Model::loadSmo(const string & path, const string & fileName, bufferUsageEnum bufferUsage) {
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
}

void Model::initBufferObj(bufferUsageEnum bufferUsage) {
	GLfloat * vertexArray = new (nothrow) GLfloat[vertexCount_*24];
	if (vertexArray == NULL) {
		cout << "Could not allocate memory for buffering the model" << endl;
		return;
	}

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
			vertexArray[count] = triangles_[i].coords[j].normal_.x;
			count++;
			vertexArray[count] = triangles_[i].coords[j].normal_.y;
			count++;
			vertexArray[count] = triangles_[i].coords[j].normal_.z;
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
			vertexArray[count] = -1.0f;
			count++;
		}
	}

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*vertexCount_*24, vertexArray, bufferUsage);
	glVertexAttribPointer(VERTEX_ATTRIBUTE, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*24, 0);
	glVertexAttribPointer(NORMAL_ATTRIBUTE, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*24,
			(const GLvoid*)(sizeof(GLfloat)*4));
	glVertexAttribPointer(COLOR0_ATTRIBUTE, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*24,
			(const GLvoid*)(sizeof(GLfloat)*7));
	glVertexAttribPointer(COLOR1_ATTRIBUTE, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*24,
			(const GLvoid*)(sizeof(GLfloat)*10));
	glVertexAttribPointer(COLOR2_ATTRIBUTE, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*24,
			(const GLvoid*)(sizeof(GLfloat)*13));
	glVertexAttribPointer(TEXTURE0_ATTRIBUTE, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*24,
			(const GLvoid*)(sizeof(GLfloat)*16));
	glVertexAttribPointer(EXTRA0_ATTRIBUTE, 1, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*24,
			(const GLvoid*)(sizeof(GLfloat)*19));
	glVertexAttribPointer(EXTRA1_ATTRIBUTE, 1, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*24,
			(const GLvoid*)(sizeof(GLfloat)*20));
	glVertexAttribPointer(EXTRA2_ATTRIBUTE, 1, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*24,
			(const GLvoid*)(sizeof(GLfloat)*21));
	glVertexAttribPointer(EXTRA3_ATTRIBUTE, 1, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*24,
			(const GLvoid*)(sizeof(GLfloat)*22));
	glVertexAttribPointer(EXTRA4_ATTRIBUTE, 1, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*24,
			(const GLvoid*)(sizeof(GLfloat)*23));

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

	delete[] vertexArray;
}

void Model::getBoneModelviewMatrices(mat4 * matrixArray, bone * pBone) {
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

			float xDiff, yDiff, zDiff,
				xDiff1 = nextFrame->xRot-previousFrame->xRot,
				yDiff1 = nextFrame->yRot-previousFrame->yRot,
				zDiff1 = nextFrame->zRot-previousFrame->zRot,

				xDiff2 = (360.0f-abs(previousFrame->xRot))-abs(nextFrame->xRot),
				yDiff2 = (360.0f-abs(previousFrame->yRot))-abs(nextFrame->yRot),
				zDiff2 = (360.0f-abs(previousFrame->zRot))-abs(nextFrame->zRot),

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
}

string Model::name() {
	return name_;
}

inline void setupVertexAttribs() {
	glVertexAttribPointer(VERTEX_ATTRIBUTE, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*24, 0);
	glVertexAttribPointer(NORMAL_ATTRIBUTE, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*24,
			(const GLvoid*)(sizeof(GLfloat)*4));
	glVertexAttribPointer(COLOR0_ATTRIBUTE, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*24,
			(const GLvoid*)(sizeof(GLfloat)*7));
	glVertexAttribPointer(COLOR1_ATTRIBUTE, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*24,
			(const GLvoid*)(sizeof(GLfloat)*10));
	glVertexAttribPointer(COLOR2_ATTRIBUTE, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*24,
			(const GLvoid*)(sizeof(GLfloat)*13));
	glVertexAttribPointer(TEXTURE0_ATTRIBUTE, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*24,
			(const GLvoid*)(sizeof(GLfloat)*16));
	glVertexAttribPointer(EXTRA0_ATTRIBUTE, 1, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*24,
			(const GLvoid*)(sizeof(GLfloat)*19));
	glVertexAttribPointer(EXTRA1_ATTRIBUTE, 1, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*24,
			(const GLvoid*)(sizeof(GLfloat)*20));
	glVertexAttribPointer(EXTRA2_ATTRIBUTE, 1, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*24,
			(const GLvoid*)(sizeof(GLfloat)*21));
	glVertexAttribPointer(EXTRA3_ATTRIBUTE, 1, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*24,
			(const GLvoid*)(sizeof(GLfloat)*22));
	glVertexAttribPointer(EXTRA4_ATTRIBUTE, 1, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*24,
			(const GLvoid*)(sizeof(GLfloat)*23));

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
}

void Model::draw(float x, float y, float z, float xRotation, float yRotation, float zRotation, float xScale,
		float yScale, float zScale, float frame, int currentAnimationId, bool skipAnimation) {
	Shader * shaderToUse;
	if (boundShader_ != NULL) shaderToUse = boundShader_; else shaderToUse = ::boundShader();

	if (shaderToUse != NULL) {
		glUseProgram(shaderToUse->program_);
		if (texture2dArrayDisabled()) {
			glBindTexture(GL_TEXTURE_2D, texture);
			shaderToUse->setUniform1(TEXCOMPAT_LOCATION, (int)textureCount);
		} else glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
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
				for (unsigned i = 0; i < bones_.size(); i++)
					setBoneRotationsFromAnimation(currentAnimationId, frame, bones_[i]);
				const int arraySize = 64;
				mat4 matrixArray[arraySize];
				getBoneModelviewMatrices(matrixArray, bones_.front());
				shaderToUse->setUniform16(EXTRA0_LOCATION, (float*)matrixArray, bones_.size());
			}

			if (vertexArrayObjectSupported()) glBindVertexArray(vao); else {
				glBindBuffer(GL_ARRAY_BUFFER, vbo);
				setupVertexAttribs();
			}
			glDrawArrays(GL_TRIANGLES, 0, vertexCount_);
			if (vertexArrayObjectSupported()) glBindVertexArray(0); else glBindBuffer(GL_ARRAY_BUFFER, 0);
		popMatrix();
	}
	if (::boundShader() != NULL) glUseProgram(::boundShader()->program_); else glUseProgram(0);
}

void Model::draw(Object & object, bool skipAnimation) {
	Shader * shaderToUse;
	if (object.boundShader_ != NULL) shaderToUse = object.boundShader_;
	else if (boundShader_ != NULL) shaderToUse = boundShader_;
	else shaderToUse = ::boundShader();

	if (shaderToUse != NULL) {
		glUseProgram(shaderToUse->program_);
		if (texture2dArrayDisabled()) {
			glBindTexture(GL_TEXTURE_2D, texture);
			shaderToUse->setUniform1(TEXCOMPAT_LOCATION, (int)textureCount);
		} else glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
		shaderToUse->setUniform1(TEXSAMPLER_LOCATION, 0);

		setMatrix(MODELVIEW_MATRIX);
		pushMatrix();
			translateMatrix(object.x_, object.y_, object.z_);
			rotateMatrix(object.xRotation_, 1.0f, 0.0f, 0.0f);
			rotateMatrix(object.yRotation_, 0.0f, 1.0f, 0.0f);
			rotateMatrix(object.zRotation_, 0.0f, 0.0f, 1.0f);
			scaleMatrix(object.xScale_, object.yScale_, object.zScale_);

			shaderToUse->setUniform16(MODELVIEW_LOCATION, getMatrix(MODELVIEW_MATRIX));
			shaderToUse->setUniform16(PROJECTION_LOCATION, getMatrix(PROJECTION_MATRIX));

			if (!skipAnimation && (bones_.size() > 0)) {
				for (unsigned i = 0; i < bones_.size(); i++)
					setBoneRotationsFromAnimation(object.currentAnimationId[i], object.frame_[i], bones_[i]);
				const int arraySize = 64;
				mat4 matrixArray[arraySize];
				getBoneModelviewMatrices(matrixArray, bones_.front());
				shaderToUse->setUniform16(EXTRA0_LOCATION, (float*)matrixArray, bones_.size());
			}

			if (vertexArrayObjectSupported()) glBindVertexArray(vao); else {
				glBindBuffer(GL_ARRAY_BUFFER, vbo);
				setupVertexAttribs();
			}
			glDrawArrays(GL_TRIANGLES, 0, vertexCount_);
			if (vertexArrayObjectSupported()) glBindVertexArray(0); else glBindBuffer(GL_ARRAY_BUFFER, 0);
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

int Model::boneId(const string & boneName) {
	for (unsigned i = 0; i < bones_.size(); i++) if (bones_[i]->name == boneName) return i;
	return -1;
}

string Model::boneName(unsigned boneId) {
	if (boneId >= bones_.size()) return "";
	return bones_[boneId]->name;
}

int Model::animationId(const string & searchName) {
	if (bones_.size() > 0) {
		for (unsigned i = 0; i < bones_.front()->animations.size(); i++)
			if (bones_.front()->animations[i].name == searchName) return i;
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

unsigned Model::vertexCount() {
	return vertexCount_;
}

int bone::animation::frameIndex(float step) {
	for (unsigned i = 0; i < frames.size(); i++) {
		if ((float)frames[i].step == step) {
			return i;
		}
	}
	return -1;
}

}
