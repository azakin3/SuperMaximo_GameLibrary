//============================================================================
// Name        : SuperMaximo_GameLibrary.cpp
// Author      : Max Foster
// Version     : 1.0
// Copyright   : http://creativecommons.org/licenses/by/3.0/
// Description : The SuperMaximo GameLibrary
//============================================================================

#include <iostream>
using namespace std;

#include "headers/SMSDL.h"
#include "headers/Input.h"
#include "headers/Utils.h"
#include "headers/Display.h"
#include "headers/classes/Object.h"
#include "headers/Audio.h"
#include "headers/classes/NetworkService.h"
#include "headers/GameHelpers.h"
#include "headers/classes/Sprite.h"
#include "headers/classes/Model.h"
#include "headers/classes/Font.h"
#include "headers/classes/Shader.h"
#include <GL/gl.h>
using namespace SuperMaximo;

int main() {
	initSDL(SDL_INIT_EVERYTHING);
	initDisplay(600, 480, 1000.0f);
	initInput();

	addSprite("background", "images/background_space.bmp", 0, 0, 600, 480);
	addSprite("laz", "images/laz.png", 0, 0, 48, 48);
	addSprite("block", "images/animtest.bmp", 0, 0, 32, 32, 10, 1, 16, 16);
	addModel("test", "models/", "teapot.obj");
	addModel("test2", "smo/cat/", "cat.smo");
	addObject("test", 0, 0, -100, model("test2"));
	addObject("block", 200, 200, -5, sprite("block"));
	addObject("block2", 0, 0, -5, sprite("block"));
	object("block")->scale(3, 3, 1, false);
	Shader * spriteShader = addShader("spriteShader", "shaders/sprite_vertex_shader.vs", "shaders/sprite_fragment_shader.fs", 1, VERTEX_ATTRIBUTE, "vertex");
	spriteShader->setUniformLocation(MODELVIEW_LOCATION, "modelviewMatrix");
	spriteShader->setUniformLocation(PROJECTION_LOCATION, "projectionMatrix");
	spriteShader->setUniformLocation(TEXSAMPLER_LOCATION, "colorMap");
	spriteShader->bind();

	Shader * modelShader = addShader("modelShader", "shaders/model_vertex_shader.vs", "shaders/model_fragment_shader.fs", 10, VERTEX_ATTRIBUTE, "vertex", NORMAL_ATTRIBUTE,
			"normal", COLOR0_ATTRIBUTE, "ambientColor", COLOR1_ATTRIBUTE, "diffuseColor", COLOR2_ATTRIBUTE, "specularColor", TEXTURE0_ATTRIBUTE, "texCoords",
			EXTRA0_ATTRIBUTE, "mtlNum", EXTRA1_ATTRIBUTE, "hasTexture", EXTRA2_ATTRIBUTE, "shininess", EXTRA3_ATTRIBUTE, "alpha");
	modelShader->setUniformLocation(MODELVIEW_LOCATION, "modelviewMatrix");
	modelShader->setUniformLocation(PROJECTION_LOCATION, "projectionMatrix");
	modelShader->setUniformLocation(TEXSAMPLER_LOCATION, "colorMap");

	Shader * skeletonShader = addShader("skeletonShader", "shaders/skeleton_vertex_shader.vs", "shaders/model_fragment_shader.fs", 10, VERTEX_ATTRIBUTE, "vertex",
			NORMAL_ATTRIBUTE, "normal", COLOR0_ATTRIBUTE, "ambientColor", COLOR1_ATTRIBUTE, "diffuseColor", COLOR2_ATTRIBUTE, "specularColor", TEXTURE0_ATTRIBUTE,
			"texCoords", EXTRA0_ATTRIBUTE, "mtlNum", EXTRA1_ATTRIBUTE, "hasTexture", EXTRA2_ATTRIBUTE, "shininess", EXTRA3_ATTRIBUTE, "alpha");
	skeletonShader->setUniformLocation(MODELVIEW_LOCATION, "modelviewMatrix");
	skeletonShader->setUniformLocation(PROJECTION_LOCATION, "projectionMatrix");
	skeletonShader->setUniformLocation(TEXSAMPLER_LOCATION, "colorMap");
	skeletonShader->setUniformLocation(EXTRA0_LOCATION, "jointModelviewMatrix");

	initFont(spriteShader);
	addFont("test", "fonts/test.ttf", 64);

	object("test")->bindShader(skeletonShader);
	object("test")->setCurrentAnimation(1);

	do {
		enableBlending();
		sprite("background")->draw(1, 1, -1000);

		object("block2")->rotate(-1, true);

		object("block2")->setX(mouseX());
		object("block2")->setY(mouseY());
		object("block")->rotate(5, true);
		object("block2")->draw();

		sprite("laz")->draw(300, 300, -10);

		object("block")->animate(0, 9);
		object("block")->draw();
		/*if (object("block")->roughBoxCollision(o("block2"))) {
			f("test")->write("Hmm...", 10, 10, 10);
			if (object("block")->boxCollision(o("block2"))) font("test")->write("Yay, it works!", 10, 100, 10);
		}*/
		copyMatrix(PERSPECTIVE_MATRIX, PROJECTION_MATRIX);
		object("test")->rotate(0.0f, 1.0f, 0.0f, true);
		object("test")->setFrame(1, true);
		object("test")->draw();

		copyMatrix(ORTHOGRAPHIC_MATRIX, PROJECTION_MATRIX);

		enableBlending(SRC_COLOR, ONE_MINUS_SRC_COLOR);
		font("test")->write("Yay, it works!", 10, 100, -10);
		font("test")->write("Hmm...", 10, 10, -100);

		refreshScreen();
	} while (!keyPressed(27) && !closeClicked());

	destroyShader("spriteShader");
	destroyShader("modelShader");
	destroyShader("skeletonShader");
	destroyShader("fontShader");
	destroyModel("test");
	destroyModel("test2");
	destroyFont("test");

	destroyAllShaders();
	destroyAllModels();
	destroyAllFonts();
	destroyAllSprites();

	quitFont();
	quitInput();
	quitDisplay();
	quitSDL();

	cout << "Quit successfully" << endl;
	return 0;
}
