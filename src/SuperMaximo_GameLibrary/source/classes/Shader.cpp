//============================================================================
// Name        : Shader.cpp
// Author      : Max Foster
// Created on  : 31 May 2011
// Version     : 1.0
// Copyright   : http://creativecommons.org/licenses/by/3.0/
// Description : SuperMaximo GameLibrary OpenGL GLSL Shader class
//============================================================================

#define GL3_PROTOTYPES 1
#include <GL3/gl3.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <cstdarg>
using namespace std;
#include "../../headers/classes/Shader.h"
#include "../../headers/Display.h"
#include "../../headers/Utils.h"
using namespace SuperMaximo;

vector<Shader *> allShaders[27];

namespace SuperMaximo {

Shader::Shader(string newName, string vertexShaderFile, string fragmentShaderFile, ...) {
	name_ = newName;
	program_ = (GLuint)NULL;
	for (int i = 0; i <= EXTRA9_LOCATION; i++) {
		uniformLocation_[i] = -1;
	}
	string text = "";
	ifstream file;
	file.open(vertexShaderFile.c_str());
	if (file.is_open()) {
		while (!file.eof()) {
			string tempStr;
			getline(file, tempStr);
			text += tempStr+"\n";
		}
		text += '\0';
		file.close();
	} else {
		cout << "Could not open file " << vertexShaderFile << endl;
		return;
	}

	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	GLchar * arr[1];
	arr[0] = (GLchar *)text.c_str();
	glShaderSource(vertexShader, 1, (const GLchar **)arr, NULL);
	glCompileShader(vertexShader);
	int success;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (success == GL_FALSE) {
		cout << "Error with compiling vertex shader" << endl;
		char log[1024];
		glGetShaderInfoLog(vertexShader, 1024, NULL, log);
		cout << log << endl;
		glDeleteShader(vertexShader);
		return;
	}

	text = "";
	file.open(fragmentShaderFile.c_str());
	if (file.is_open()) {
		while (!file.eof()) {
			string tempStr;
			getline(file, tempStr);
			text += tempStr+"\n";
		}
		text += '\0';
		file.close();
	} else {
		glDeleteShader(vertexShader);
		cout << "Could not open file " << fragmentShaderFile << endl;
		return;
	}

	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	arr[0] = (GLchar *)text.c_str();
	glShaderSource(fragmentShader, 1, (const GLchar **)arr, NULL);
	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (success == GL_FALSE) {
		cout << "Error with compiling fragment shader" << endl;
		char log[1024];
		glGetShaderInfoLog(fragmentShader, 1024, NULL, log);
		cout << log << endl;
		glDeleteShader(fragmentShader);
		glDeleteShader(vertexShader);
		return;
	}

	program_ = glCreateProgram();
	glAttachShader(program_, vertexShader);
	glAttachShader(program_, fragmentShader);

	va_list attributes;
	va_start(attributes, fragmentShaderFile);
	int num = va_arg(attributes, int);
	for (int i = 0; i < num; i++) {
		int index = va_arg(attributes, int);
		char * attr = va_arg(attributes, char *);
		glBindAttribLocation(program_, index, attr);
	}
	va_end(attributes);

	glLinkProgram(program_);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	glGetProgramiv(program_, GL_LINK_STATUS, &success);
	if (success == GL_FALSE) {
		cout << "Error with linking shader program" << endl;
		char log[1024];
		glGetProgramInfoLog(program_, 1024, NULL, log);
		cout << log << endl;
		glDeleteProgram(program_);
		return;
	}
}

Shader::Shader(string newName, string vertexShaderFile, string fragmentShaderFile, vector<int> enums, vector<char *>attributeNames) {
	name_ = newName;
	program_ = (GLuint)NULL;
	for (short i = 0; i <= EXTRA9_LOCATION; i++) uniformLocation_[i] = -1;
	string text = "";
	ifstream file;
	file.open(vertexShaderFile.c_str());
	if (file.is_open()) {
		while (!file.eof()) {
			string tempStr;
			getline(file, tempStr);
			text += tempStr+"\n";
		}
		text += '\0';
		file.close();
	} else {
		cout << "Could not open file " << vertexShaderFile << endl;
		return;
	}

	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	GLchar * arr[1];
	arr[0] = (GLchar *)text.c_str();
	glShaderSource(vertexShader, 1, (const GLchar **)arr, NULL);
	glCompileShader(vertexShader);
	int success;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (success == GL_FALSE) {
		cout << "Error with compiling vertex shader" << endl;
		char log[1024];
		glGetShaderInfoLog(vertexShader, 1024, NULL, log);
		cout << log << endl;
		glDeleteShader(vertexShader);
		return;
	}

	text = "";
	file.open(fragmentShaderFile.c_str());
	if (file.is_open()) {
		while (!file.eof()) {
			string tempStr;
			getline(file, tempStr);
			text += tempStr+"\n";
		}
		text += '\0';
		file.close();
	} else {
		glDeleteShader(vertexShader);
		cout << "Could not open file " << fragmentShaderFile << endl;
		return;
	}

	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	arr[0] = (GLchar *)text.c_str();
	glShaderSource(fragmentShader, 1, (const GLchar **)arr, NULL);
	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (success == GL_FALSE) {
		cout << "Error with compiling fragment shader" << endl;
		char log[1024];
		glGetShaderInfoLog(fragmentShader, 1024, NULL, log);
		cout << log << endl;
		glDeleteShader(fragmentShader);
		glDeleteShader(vertexShader);
		return;
	}

	program_ = glCreateProgram();
	glAttachShader(program_, vertexShader);
	glAttachShader(program_, fragmentShader);

	for (unsigned i = 0; i < enums.size(); i++) glBindAttribLocation(program_, enums[i], attributeNames[i]);

	glLinkProgram(program_);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	glGetProgramiv(program_, GL_LINK_STATUS, &success);
	if (success == GL_FALSE) {
		cout << "Error with linking shader program" << endl;
		char log[1024];
		glGetProgramInfoLog(program_, 1024, NULL, log);
		cout << log << endl;
		glDeleteProgram(program_);
		return;
	}
}

Shader::~Shader() {
	glDeleteProgram(program_);
}

string Shader::name() {
	return name_;
}

void Shader::bind() {
	bindShader(this);
}

void Shader::use() {
	glUseProgram(program_);
}

GLuint Shader::program() {
	return program_;
}

GLint Shader::setUniformLocation(shaderLocationEnum dstLocation, string locationName) {
	uniformLocation_[dstLocation] = glGetUniformLocation(program_, locationName.c_str());
	return uniformLocation_[dstLocation];
}

GLint Shader::uniformLocation(shaderLocationEnum location) {
	return uniformLocation_[location];
}

void Shader::setUniform1(shaderLocationEnum location, GLfloat * data, unsigned count) {
	glUniform1fv(uniformLocation_[location], count, data);
}

void Shader::setUniform2(shaderLocationEnum location, GLfloat * data, unsigned count) {
	glUniform2fv(uniformLocation_[location], count, data);
}

void Shader::setUniform3(shaderLocationEnum location, GLfloat * data, unsigned count) {
	glUniform3fv(uniformLocation_[location], count, data);
}

void Shader::setUniform4(shaderLocationEnum location, GLfloat * data, unsigned count) {
	glUniform4fv(uniformLocation_[location], count, data);
}

void Shader::setUniform9(shaderLocationEnum location, GLfloat * data, unsigned count) {
	glUniformMatrix3fv(uniformLocation_[location], count, GL_FALSE, data);
}

void Shader::setUniform16(shaderLocationEnum location, GLfloat * data, unsigned count) {
	glUniformMatrix4fv(uniformLocation_[location], count, GL_FALSE, data);
}

void Shader::setUniform1(shaderLocationEnum location, int * data, unsigned count) {
	glUniform1iv(uniformLocation_[location], count, data);
}

void Shader::setUniform2(shaderLocationEnum location, int * data, unsigned count) {
	glUniform2iv(uniformLocation_[location], count, data);
}

void Shader::setUniform3(shaderLocationEnum location, int * data, unsigned count) {
	glUniform3iv(uniformLocation_[location], count, data);
}

void Shader::setUniform4(shaderLocationEnum location, int * data, unsigned count) {
	glUniform4iv(uniformLocation_[location], count, data);
}

void Shader::setUniform1(shaderLocationEnum location, GLfloat data) {
	glUniform1f(uniformLocation_[location], data);
}

void Shader::setUniform2(shaderLocationEnum location, GLfloat data1, GLfloat data2) {
	glUniform2f(uniformLocation_[location], data1, data2);
}

void Shader::setUniform3(shaderLocationEnum location, GLfloat data1, GLfloat data2, GLfloat data3) {
	glUniform3f(uniformLocation_[location], data1, data2, data3);
}

void Shader::setUniform4(shaderLocationEnum location, GLfloat data1, GLfloat data2, GLfloat data3, GLfloat data4) {
	glUniform4f(uniformLocation_[location], data1, data2, data3, data4);
}

void Shader::setUniform1(shaderLocationEnum location, int data) {
	glUniform1i(uniformLocation_[location], data);
}

void Shader::setUniform2(shaderLocationEnum location, int data1, int data2) {
	glUniform2i(uniformLocation_[location], data1, data2);
}

void Shader::setUniform3(shaderLocationEnum location, int data1, int data2, int data3) {
	glUniform3i(uniformLocation_[location], data1, data2, data3);
}

void Shader::setUniform4(shaderLocationEnum location, int data1, int data2, int data3, int data4) {
	glUniform4i(uniformLocation_[location], data1, data2, data3, data4);
}

Shader * shader(string searchName) {
	int letter = numCharInAlphabet(searchName[0]);
	Shader * returnShader = NULL;
	if (allShaders[letter].size() > 0) {
		for (unsigned int i = 0; i < allShaders[letter].size(); i++) {
			if (allShaders[letter][i]->name() == searchName) {
				returnShader = allShaders[letter][i];
				break;
			}
		}
	}
	return returnShader;
}

Shader * addShader(string newName, string vertexShaderFile, string fragmentShaderFile, ...) {
	int letter = numCharInAlphabet(newName[0]);
	vector<int> enums;
	vector<char *>attributeNames;
	va_list attributes;
	va_start(attributes, fragmentShaderFile);
	int count = va_arg(attributes, int);
	for (int i = 0; i < count; i++) {
		int index = va_arg(attributes, int);
		enums.push_back(index);
		char * attr = va_arg(attributes, char *);
		attributeNames.push_back(attr);
	}
	va_end(attributes);
	Shader * newShader = new Shader(newName, vertexShaderFile, fragmentShaderFile, enums, attributeNames);
	allShaders[letter].push_back(newShader);
	return newShader;
}

void destroyShader(string searchName) {
	int letter = numCharInAlphabet(searchName[0]);
	if (allShaders[letter].size() > 0) {
		for (unsigned int i = 0; i < allShaders[letter].size(); i++) {
			if (allShaders[letter][i]->name() == searchName) {
				delete allShaders[letter][i];
				allShaders[letter].erase(allShaders[letter].begin()+i);
				break;
			}
		}
	}
}

void destroyAllShaders() {
	for (int i = 0; i < 27; i++) {
		if (allShaders[i].size() > 0) {
			for (unsigned int j = 0; j < allShaders[i].size(); j++) delete allShaders[i][j];
			allShaders[i].clear();
		}
	}
}

}
