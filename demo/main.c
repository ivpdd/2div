#include <stdio.h>
#include <math.h>
#include <time.h>
#include "../2div.h"

int main() {
	srand(time(NULL));
	divinit();
	divwin win = divwincreate(640,640,"window123!@#$ф",true);
	divfnt font = divloadttf("test.ttf");
	divpic png = divloadpic("test.png");
	divsnd sound;
	divloadsnd(&sound,"test.mp3");
	divplaysnd(&sound);
	int circlespin = 0;
	Body *dynamictux = divphysaddbodycircle((Vector2){100,100},50,90,1,true);
	Body *dynamictux1 = divphysaddbodycircle((Vector2){120,100},50,90,1,false);
	Body *ground = divphysaddbodybox((Vector2){0,600},(Vector2){640,100},0,1,false);
	while (!glfwWindowShouldClose(win)) {
		divwinclear(win,(RGB){255,255,255});

		divdrawrect(win,
		(Vector2){10,10},//position
		(Vector2){200,200},//size
		(RGBA){255,0,0,255},//color
		circlespin);//rotation

		divdrawtri(win,
		(Vector2){200,400},//position
		(Vector2){200,510},//position
		(Vector2){310,400},//position
		(RGBA){255,255,0,255},//color
		png,//picture(0 for filled)
		(Vector2){0,0},(Vector2){0,0},(Vector2){0,0});/*texcoords(0,0 for all points default) so with texcoords you can offset the picture 100 is 1 picture scale*/
		int offset = circlespin/3.6;
		divdrawtri(win,
		(Vector2){200,200},//position
		(Vector2){200,310},//position
		(Vector2){310,200},//position
		(RGBA){255,255,0,255},//color
		png,//picture(0 for filled)
		(Vector2){offset,offset},(Vector2){-offset,offset},(Vector2){offset,-offset});

		divdrawtri(win,
		(Vector2){400,400},
		(Vector2){400,510},
		(Vector2){510,400},
		(RGBA){255,255,0,255},
		png,
		(Vector2){offset,offset},(Vector2){offset,offset},(Vector2){offset,offset});

		divdrawtri(win,
		(Vector2){510,310},
		(Vector2){400,310},
		(Vector2){510,200},
		(RGBA){255,255,0,255},
		png,
		(Vector2){offset,0},(Vector2){offset,0},(Vector2){offset,0});

		divdrawtri(win,
		(Vector2){400,200},
		(Vector2){400,310},
		(Vector2){510,200},
		(RGBA){255,255,0,255},
		png,
		(Vector2){0,offset},(Vector2){0,offset},(Vector2){0,offset});


		divdrawcircle(win,
		(Vector2){500,500},
		(Vector2){100,100},
		(RGBA){255,0,255,255},
		circlespin/60,circlespin,
		0);

		divdrawcircle(win,
		(Vector2){500,500},//position
		(Vector2){100,100},//size
		(RGBA){255,0,255,255},//color
		circlespin/60,circlespin,//segments,rotation
		png);//picture(0 for filled)
		circlespin+= 5;
		if (circlespin == 360) {
			circlespin = 0;
		}

		divdrawpic(win,
		png,//picture
		(Vector2){300,300},//position
		(Vector2){100,200},//size
		(RGBA){0,255,0,255},360-circlespin);//color,rotation

		for (int i = 100; i < 200; i++) {
			for (int a = 100; a < 200; a++) {
				divdrawpix(win,
				(Vector2){i,a},// position
				(RGBA){i,a,divwingetkeypress(win,'E')*255,255});//color
			}
		}

		Vector2 pos = divwingetcursorpos(win);

		RGB s = divwingetpixel(win,pos);
		char buf[32];
		snprintf(buf, sizeof(buf), "(%d, %d, %d)", s.r, s.g, s.b);
		divdrawtext(win,
		buf,//text
		(Vector2){100,100},//position
		110,//size
		(RGBA){0,255,0,255},//color
		font);//font

		divdrawpic(win,
		png,
		(Vector2){100,100},
		(Vector2){100,100},
		(RGBA){0,255,255,200},(atan2(pos.y-150,pos.x-150)*(180.0 / M_PI))+90);

		divdrawpic(win, png,
		(Vector2){dynamictux->pos.x, dynamictux->pos.y},
		(Vector2){dynamictux->size.x,dynamictux->size.y},
		(RGBA){255,255,255,255}, dynamictux->rot);

		divdrawpic(win, png,
		(Vector2){dynamictux1->pos.x, dynamictux1->pos.y},
		(Vector2){dynamictux1->size.x,dynamictux1->size.y},
		(RGBA){255,255,255,255}, dynamictux1->rot);

		divdrawrect(win,
		(Vector2){0,600},
		(Vector2){640,100},
		(RGBA){100,100,100,255},
		0);

		divwinupdate(win);
	}
	divwinclose(win);
	divstop();
	return 0;
} 