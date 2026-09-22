#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ext/Chipmunk2D/include/chipmunk/chipmunk.h"
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

typedef struct {
	Vector2 pos;
	Vector2 size;
	Vector2 vel;
	int rotvel;
	int rot;
	int mass;
	bool dynamic;//if dynamic is false then body wont fall or move in anyway,it will interact with other bodies though
	cpBody *body;
} Body;



#define ROT_X(x,y,cx,cy,c,s) ((int)(cx + ((x)-cx)*(c) - ((y)-cy)*(s)))
#define ROT_Y(x,y,cx,cy,c,s) ((int)(cy + ((x)-cx)*(s) + ((y)-cy)*(c)))

struct FONScontext* fs = NULL;
ma_engine engine;
Body bodies[256];
int bodyCount = 0;
cpSpace *space = NULL;
/*
N - needed - functions that need to be called
D - do - functions that do something
G - get - functions that checks then return something
L - load - functions that load then return something
*/

/////\/\\//\/\\\\/\/\\\//\\/\\\\//\\\///\/\\\//\\/\\\\//\\\///\/\\\//\\/\\\\//\\\///\/\\
//N type functions/////\\//\/\//\\//\\\/\\//\\\\/\\\//\\\\\\\/\\//\\\\/\\\//\\\\\\\/\\//
//////\/\//\\/\/\\\/\\//\\/\\\/\\//\\/\\\//\\\\/\\\//\\\\\\\/\\//\\\\/\\\//\\\\\\\/\\//\

void divinit() {
	glfwInit();
	glfwSwapInterval(1);
	ma_engine_init(NULL,&engine);
	space = cpSpaceNew();
	cpSpaceSetGravity(space,cpv(0,200));
}

void divstop() {
	glfwTerminate();
	ma_engine_uninit(&engine);
	cpSpaceFree(space);
}

divwin divwincreate(int w,int h,const char* title,bool decorated,bool transparency,bool fullscreen) {
	glfwWindowHint(GLFW_DECORATED,decorated);
	glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER,transparency);
	divwin window;
	if (fullscreen) {
		window = glfwCreateWindow(w,h,title,glfwGetPrimaryMonitor(),NULL);
	} else {
		window = glfwCreateWindow(w,h,title,NULL,NULL);
	}
	if (!window) {
		return NULL;
	}
	glfwMakeContextCurrent(window);
	fs = glfonsCreate(512,512,FONS_ZERO_TOPLEFT);
	return window;
}

void divwinclear(divwin window,RGB color) {
	int width = 0;
	int height = 0;
	glfwMakeContextCurrent(window);
	glfwGetFramebufferSize(window,&width,&height);
	
	glViewport(0,0,width,height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0,width,height,0.0,-1.0,1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	glClearColor((float)color.r/255,(float)color.g/255,(float)color.b/255,1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glFlush();
}

void divwinclose(divwin window) {
	glfwDestroyWindow(window);
}

void divwinupdate(divwin window) {
	glfwPollEvents();

	cpSpaceStep(space,1.0f/60.0f);
	for (int i = 0; i < bodyCount; i++) {
		cpVect p = cpBodyGetPosition(bodies[i].body);
		cpVect r = cpBodyGetRotation(bodies[i].body);
		//printf("n: %d x: %d y: %d \n",i,(int)p.x,(int)p.y);
		bodies[i].pos.x = p.x;
		bodies[i].pos.y = p.y;
		bodies[i].rot = (int)(atan2f(r.y,r.x) * (180.0 / M_PI));
	}

	glfwSwapBuffers(window);
}

/////\/\\//\/\\\\/\/\\\//\\/\\\\//\\\///\/\\\//\\/\\\\//\\\///\/\\\//\\/\\\\//\\\///\/\\
//D type functions/////\\//\/\//\\//\\\/\\//\\\\/\\\//\\\\\\\/\\//\\\\/\\\//\\\\\\\/\\//
//////\/\//\\/\/\\\/\\//\\/\\\/\\//\\/\\\//\\\\/\\\//\\\\\\\/\\//\\\\/\\\//\\\\\\\/\\//\

void divdrawrect(divwin window,Vector2 pos,Vector2 size,RGBA color,int rotation) {
	glfwMakeContextCurrent(window);
	glColor4ub(color.r,color.g,color.b,color.a);
	float cx = pos.x + size.x/2.f,cy = pos.y + size.y/2.f;
	float r = rotation * M_PI/180.f;
	float c = cosf(r);
	float s = sinf(r);
	glBegin(GL_QUADS);
	//really scrambled code TODO: unscramble it somehow and make it readable
	glVertex2i(ROT_X(pos.x,pos.y,cx,cy,c,s),ROT_Y(pos.x,pos.y,cx,cy,c,s));
	glVertex2i(ROT_X(pos.x + size.x,pos.y,cx,cy,c,s),ROT_Y(pos.x + size.x,pos.y,cx,cy,c,s));
	glVertex2i(ROT_X(pos.x + size.x,pos.y + size.y,cx,cy,c,s),ROT_Y(pos.x + size.x,pos.y + size.y,cx,cy,c,s));
	glVertex2i(ROT_X(pos.x,pos.y + size.y,cx,cy,c,s),ROT_Y(pos.x,pos.y + size.y,cx,cy,c,s));
	glEnd();
}

void divdrawtri(divwin window,Vector2 xy1,Vector2 xy2,Vector2 xy3,
	RGBA color,divpic img,Vector2 texcoord1,Vector2 texcoord2,Vector2 texcoord3) {

	glfwMakeContextCurrent(window);
	if (img != 0) {
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D,img);
	}
	glColor4ub(color.r,color.g,color.b,color.a);
	glBegin(GL_TRIANGLES);
	if (img != 0) {glTexCoord2f(0.0f+texcoord1.x/100.0f,0.0f+texcoord1.y/100.0f);}
	glVertex2i(xy1.x,xy1.y);
	if (img != 0) {glTexCoord2f(1.0f+texcoord2.x/100.0f,0.0f+texcoord2.y/100.0f);}
	glVertex2i(xy2.x,xy2.y);
	if (img != 0) {glTexCoord2f(0.0f+texcoord3.x/100.0f,1.0f+texcoord3.y/100.0f);}
	glVertex2i(xy3.x,xy3.y);

	glEnd();
	if (img != 0){glDisable(GL_TEXTURE_2D);}
}

void divdrawcircle(divwin window,
	Vector2 pos,Vector2 size,RGBA color,
	int segments,int rotation,divpic img) {

	if (img != 0) {
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D,img);
	}

	glfwMakeContextCurrent(window);
	glColor4ub(color.r,color.g,color.b,color.a);
	float cx = pos.x + size.x/2.f,cy = pos.y + size.y/2.f;
	float rx = size.x/2.f,ry = size.y/2.f;
	float r = rotation * M_PI/180.f;
	float c = cosf(r);
	float s = sinf(r);
	glBegin(GL_TRIANGLE_FAN);
	if (img != 0) {glTexCoord2f(0.5f,0.5f);}
	glVertex3i((int)cx,(int)cy,0);
	//TODO: unscramble this
	for (int i = 0; i <= segments; i++) {
		float phi = 2.f * M_PI * i / segments;
		float x = cx + rx*cosf(phi),y = cy + ry*sinf(phi);
		if (img != 0) {
			glTexCoord2f(0.5f + 0.5f*cosf(phi),0.5f + 0.5f*sinf(phi));
		}
		glVertex3i(ROT_X(x,y,cx,cy,c,s),ROT_Y(x,y,cx,cy,c,s),0);
	}
	glEnd();
	if (img != 0) {
		glDisable(GL_TEXTURE_2D);
	}
}

void divdrawpix(divwin window,Vector2 pos,
	RGBA color) {

	glfwMakeContextCurrent(window);
	glColor4ub(color.r,color.g,color.b,color.a);
	glBegin(GL_POINTS);
	glVertex2i(pos.x,pos.y);
	glEnd();
}

void divdrawtext(divwin window,char *text,Vector2 pos,int size,
	RGBA color,
	divfnt font) {

	glfwMakeContextCurrent(window);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	fonsSetFont(fs,font);
	fonsSetSize(fs,(float)size);
	fonsSetColor(fs,glfonsRGBA(color.r,color.g,color.b,color.a));
	fonsDrawText(fs,pos.x,pos.y,text,NULL);
}

void divdrawpic(divwin window,divpic tex,
	Vector2 pos,Vector2 size,
	RGBA color,int rotation) {

	glfwMakeContextCurrent(window);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glColor4ub(color.r,color.g,color.b,color.a);
	glBindTexture(GL_TEXTURE_2D,tex);
	glBegin(GL_QUADS);
	//same here
	float cx = pos.x + size.x/2.f,cy = pos.y + size.y/2.f;
	float r = rotation * M_PI/180.f;
	float c = cosf(r);
	float s = sinf(r);
	glTexCoord2i(0,1);
	glVertex2i(ROT_X(pos.x,pos.y,cx,cy,c,s),ROT_Y(pos.x,pos.y,cx,cy,c,s));
	glTexCoord2i(1,1);
	glVertex2i(ROT_X(pos.x + size.x,pos.y,cx,cy,c,s),ROT_Y(pos.x + size.x,pos.y,cx,cy,c,s));
	glTexCoord2i(1,0);
	glVertex2i(ROT_X(pos.x + size.x,pos.y + size.y,cx,cy,c,s),ROT_Y(pos.x + size.x,pos.y + size.y,cx,cy,c,s));
	glTexCoord2i(0,0);
	glVertex2i(ROT_X(pos.x,pos.y + size.y,cx,cy,c,s),ROT_Y(pos.x,pos.y + size.y,cx,cy,c,s));

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

void divwinchangeopacity(divwin window,double opacity) {
	glfwSetWindowOpacity(window,(float)opacity);
}

Body* divphysaddbodybox(Vector2 pos,Vector2 size,int rot,int mass,bool dynamic) {
	Body *b = &bodies[bodyCount++];
	b->pos = pos;
	b->size = size;
	b->mass = mass;
	b->dynamic = dynamic;
	b->rot = rot;
	cpFloat m,i;
	if (dynamic) {
		m = (cpFloat)mass;
		i = cpMomentForBox(m,size.x,size.y);
	} else {
		m = 0.0f;
		i = INFINITY;
	}

	b->body = cpBodyNew(m,i);

	if (!dynamic) cpBodySetType(b->body,CP_BODY_TYPE_STATIC);

	cpBodySetPosition(b->body,cpv(pos.x,pos.y));
	cpBodySetAngle(b->body,rot * (M_PI / 180.0));
	cpSpaceAddBody(space,b->body);
	cpShape *physshape = cpBoxShapeNew(b->body,size.x,size.y,0.0f);
	cpSpaceAddShape(space,physshape);

	return b;
}

Body* divphysaddbodycircle(Vector2 pos,int radius,int rot,int mass,bool dynamic) {
	Body *b = &bodies[bodyCount++];
	b->pos = pos;
	b->size = (Vector2){radius*2,radius*2};
	b->mass = mass;
	b->dynamic = dynamic;
	b->rot = rot;
	cpFloat m,i;
	if (dynamic) {
		m = (cpFloat)mass;
		i = cpMomentForCircle(m,(cpFloat)radius,0.0f,cpvzero);
	} else {
		m = 0.0f;
		i = INFINITY;
	}

	b->body = cpBodyNew(m,i);

	if (!dynamic) cpBodySetType(b->body,CP_BODY_TYPE_STATIC);

	cpBodySetPosition(b->body,cpv(pos.x,pos.y));
	cpBodySetAngle(b->body,rot * (M_PI / 180.0));
	cpSpaceAddBody(space,b->body);
	cpShape *physshape = cpCircleShapeNew(b->body,radius,cpvzero);
	cpSpaceAddShape(space,physshape);

	return b;
}

void divphyssetgrav(Vector2 gravity) {
	cpSpaceSetGravity(space,cpv(gravity.x,gravity.y));
}

/////\/\\//\/\\\\/\/\\\//\\/\\\\//\\\///\/\\\//\\/\\\\//\\\///\/\\\//\\/\\\\//\\\///\/\\
//G type functions/////\\//\/\//\\//\\\/\\//\\\\/\\\//\\\\\\\/\\//\\\\/\\\//\\\\\\\/\\//
//////\/\//\\/\/\\\/\\//\\/\\\/\\//\\/\\\//\\\\/\\\//\\\\\\\/\\//\\\\/\\\//\\\\\\\/\\//\

Vector2 divwingetcursorpos(divwin window) {
	double x = 0,y = 0;
	glfwGetCursorPos(window,&x,&y);
	return (Vector2){(int)x,(int)y};
}

RGB divwingetpixel(divwin window,Vector2 pos) {
	glfwMakeContextCurrent(window);
	int width,height;
	glfwGetWindowSize(window,&width,&height);
	int x = (int)pos.x;
	int y = height - (int)pos.y - 1;
	glPixelStorei(GL_PACK_ALIGNMENT,1);
	unsigned char data[3] = {0};
	glReadPixels(x,y,1,1,GL_RGB,GL_UNSIGNED_BYTE,data);
	return (RGB){ data[0],data[1],data[2] };
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

/////\/\\//\/\\\\/\/\\\//\\/\\\\//\\\///\/\\\//\\/\\\\//\\\///\/\\\//\\/\\\\//\\\///\/\\
//L type functions/////\\//\/\//\\//\\\/\\//\\\\/\\\//\\\\\\\/\\//\\\\/\\\//\\\\\\\/\\//
//////\/\//\\/\/\\\/\\//\\/\\\/\\//\\/\\\//\\\\/\\\//\\\\\\\/\\//\\\\/\\\//\\\\\\\/\\//\

divfnt divloadttf(char *ttfpath) {
	divfnt fontNormal = fonsAddFont(fs,"sans",ttfpath);
	return fontNormal;
}

divfnt divloadttfmem(unsigned char *data,int len) {
	divfnt fontNormal = fonsAddFontMem(fs,"sans",data,len,0);
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

divpic divloadpicmem(unsigned char *data,int len) {
	divpic tex = SOIL_load_OGL_texture_from_memory(data,len,SOIL_LOAD_AUTO,SOIL_CREATE_NEW_ID,SOIL_FLAG_INVERT_Y);
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
