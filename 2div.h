#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ext/soil/src/SOIL/SOIL.h"
#define FONTSTASH_IMPLEMENTATION
#include "ext/fontstash/fontstash.h"
#define GLFONTSTASH_IMPLEMENTATION
#include "ext/fontstash/glfontstash.h"
#define MINIAUDIO_IMPLEMENTATION
#include "ext/miniaudio.h"

typedef GLFWwindow* divwin;
typedef unsigned int divpic;
typedef unsigned int divfnt;
typedef ma_sound divsnd;

typedef struct {
	int x;
	int y;
} Vector2;

typedef struct {
	int r;
	int g;
	int b;
	int a;
} RGBA;

typedef struct {
	int r;
	int g;
	int b;
} RGB;

struct FONScontext* fs = NULL;
ma_engine engine;

/*
N - needed
D - do
G - get
L - load
*/

// N type functions

void divinit() {
	glfwInit();
	glfwSwapInterval(1);
	ma_engine_init(NULL, &engine);
}

void divstop() {
	glfwTerminate();
	ma_engine_uninit(&engine);
}

divwin divwincreate(int w, int h, const char* title, bool decorated) {
	glfwWindowHint(GLFW_DECORATED, decorated);
	divwin window = glfwCreateWindow(w, h, title, NULL, NULL);
	if (!window) {
		return NULL;
	}
	glfwMakeContextCurrent(window);
	fs = glfonsCreate(512, 512, FONS_ZERO_TOPLEFT);
	return window;
}

void divwinclear(divwin window, RGB color) {
	int width = 0, height = 0;
	glfwMakeContextCurrent(window);
	glfwGetFramebufferSize(window, &width, &height);
	
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, width, height, 0.0, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	glClearColor((float)color.r/255, (float)color.g/255, (float)color.b/255, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glFlush();
}

void divwinclose(divwin window) {
	glfwDestroyWindow(window);
}

void divwinupdate(divwin window) {
	glfwSwapBuffers(window);
	glfwPollEvents();
}

//D type functions

void divdrawrect(divwin window,Vector2 pos,Vector2 size,
	RGB color) {

	glfwMakeContextCurrent(window);
	glColor3ub(color.r,color.g,color.b);
	glBegin(GL_QUADS);
	glVertex2i(pos.x,pos.y);
	glVertex2i(pos.x + size.x,pos.y);
	glVertex2i(pos.x + size.x,pos.y + size.y);
	glVertex2i(pos.x,pos.y + size.y);
	glEnd();
}

void divdrawtri(divwin window,Vector2 xy1,Vector2 xy2,Vector2 xy3,
	RGB color) {

	glfwMakeContextCurrent(window);
	glColor3ub(color.r,color.g,color.b);
	glBegin(GL_TRIANGLES);
	glVertex2i(xy1.x,xy1.y);
	glVertex2i(xy2.x,xy2.y);
	glVertex2i(xy3.x,xy3.y);
	glEnd();
}

void divdrawpix(divwin window,Vector2 pos,
	RGB color) {

	glfwMakeContextCurrent(window);
	glColor3ub(color.r,color.g,color.b);
	glBegin(GL_POINTS);
	glVertex2i(pos.x,pos.y);
	glEnd();
}

void divdrawtext(divwin window,char *text,Vector2 pos,int size,
	RGBA color,
	divfnt font) {

	glfwMakeContextCurrent(window);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	fonsSetFont(fs,font);
	fonsSetSize(fs,(float)size);
	fonsSetColor(fs,glfonsRGBA(color.r,color.g,color.b,color.a));
	fonsDrawText(fs,pos.x,pos.y,text,NULL);
}

void divdrawpic(divwin window,divpic tex,
	Vector2 pos,Vector2 size) {

	glfwMakeContextCurrent(window);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glColor4ub(255,255,255,255);
	glBindTexture(GL_TEXTURE_2D,tex);
	glBegin(GL_QUADS);
	glTexCoord2i(0,1); glVertex2i(pos.x,pos.y);
	glTexCoord2i(1,1); glVertex2i(pos.x + size.x,pos.y);
	glTexCoord2i(1,0); glVertex2i(pos.x + size.x,pos.y + size.y);
	glTexCoord2i(0,0); glVertex2i(pos.x,pos.y + size.y);
	glEnd();
	glDisable(GL_TEXTURE_2D);
}

void divplaysnd(divsnd* sound) {
	ma_sound_start(sound);
}

void divwinchangepos(divwin window,Vector2 pos) {
	glfwSetWindowPos(window,pos.x,pos.y);
}

void divwinchangesize(divwin window,Vector2 size) {
	glfwSetWindowSize(window,size.x,size.y);
}

//G type functions

Vector2 divgetcursorpos(divwin window) {
	double x = 0,y = 0;
	glfwGetCursorPos(window,&x,&y);
	return (Vector2){(int)x,(int)y};
}

bool divwingetkeypress(divwin window,char key) {
	int code = 0;
	if (key >= 'a' && key <= 'z') code = GLFW_KEY_A + (key - 'a');
	else if (key >= 'A' && key <= 'Z') code = GLFW_KEY_A + (key - 'A');
	else if (key >= '0' && key <= '9') code = GLFW_KEY_0 + (key - '0');
	else if (key == ' ') code = GLFW_KEY_SPACE;
	else return false;

	return glfwGetKey(window,code) == GLFW_PRESS;
}

bool divwingetlmb(divwin window) {
	return glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_LEFT);
}

bool divwingetrmb(divwin window) {
	return glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_RIGHT);
}

//L type functions

divfnt divloadttf(char *ttfpath) {
	divfnt fontNormal = fonsAddFont(fs,"sans",ttfpath);
	return fontNormal;
}

divpic divloadpic(char* picpath) {
	divpic tex = SOIL_load_OGL_texture(picpath,SOIL_LOAD_AUTO,SOIL_CREATE_NEW_ID,SOIL_FLAG_INVERT_Y);
	glBindTexture(GL_TEXTURE_2D,tex);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
	return tex;
}

void divloadsnd(divsnd* sound,char* sndpath) {
	ma_sound_init_from_file(&engine,sndpath,0,NULL,NULL,sound);
}