#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdlib.h>
#include <string.h>
//タッチパネル制御
//黄色スタートで
//touch			x+ y+ x- y-
//GPIO_NUM	 0  1  2  3
#define GPIO_0 0
#define GPIO_1 1
#define GPIO_2 2
#define GPIO_3 3
//キャリブレーションデータ
//toriaezu
#define MIN_X 88
#define MAX_X 810
#define MIN_Y 190 
#define MAX_Y 720
#define WAIT_US 10000

int display_width,display_height;
void calibratedMouseMove(Display *display,int x,int y);
void mouseMove(Display *display,int x,int y);
void mouseClick(int button); 

void getTouch(int* x,int* y);
int formatADCdata(unsigned char raw_data[]);

int main(int argc , char* argv[]) {
  Display *display;
  int i; int x,y; 
	int prex,prey;
  if(wiringPiSetup() == -1){ perror("Setup Error"); return 1; }
  wiringPiSPISetup(0,500000);
	pinMode(GPIO_0,OUTPUT); pinMode(GPIO_1,OUTPUT);
	pinMode(GPIO_2,OUTPUT); pinMode(GPIO_3,OUTPUT);

  /* ディスプレイ・サーバに接続 */
  display = XOpenDisplay(NULL);
  if(display == NULL){
    fprintf(stderr, "Cannot open display.\n");
    exit(1);
  }
  usleep(50);
  //キャリブレーション用データ
  display_width=DisplayWidth(display,0);
  display_height=DisplayHeight(display,0);
	printf("%d x %d \n",display_width,display_height);
  while(1){
    int x=0,y=0;
		getTouch(&x,&y);
		if((x>3&&y>3)&&abs(prex-x)<5&&abs(prey-y)<5){
			calibratedMouseMove(display,x,y);
		}
		printf("%5d %5d\n",x,y);
		prex=x,prey=y;
    XFlush(display);
  }
  //mouseClick(Button1);
  XFlush(display);
  /* ディスプレイ・サーバとの接続を解除 */
  XCloseDisplay(display);
  return 0;
}
void getTouch(int* x,int* y){
  unsigned char buf[3];
	pinMode(GPIO_0,OUTPUT); pinMode(GPIO_1,INPUT); 
	pinMode(GPIO_2,OUTPUT); pinMode(GPIO_3,INPUT);
	digitalWrite(GPIO_0,1); digitalWrite(GPIO_2,0);
	usleep(WAIT_US);
	buf[0]=0x06;buf[1]=0x40;buf[2]=0x00;
	wiringPiSPIDataRW(0,buf,3);
	*x=formatADCdata(buf);
	digitalWrite(GPIO_0,0);
	usleep(WAIT_US);
	//y
	pinMode(GPIO_0,INPUT); pinMode(GPIO_1,OUTPUT); 
	pinMode(GPIO_2,INPUT); pinMode(GPIO_3,OUTPUT);
	digitalWrite(GPIO_1,1); digitalWrite(GPIO_3,0);
	usleep(WAIT_US);
	buf[0]=0x06;buf[1]=0x00;buf[2]=0x00;
	wiringPiSPIDataRW(0,buf,3);
	*y=formatADCdata(buf);
	digitalWrite(GPIO_1,0);
	usleep(WAIT_US);
  return;
}

int formatADCdata(unsigned char raw_data[]){
	int value=0;	
	value=(raw_data[1]&0x0f)<< 8;
	value=raw_data[2]+value;
	value>>=2;
	return value;
}
///////////////////////////////////////////

void calibratedMouseMove(Display *display,int x,int y){
  int cx = display_width*((double)x-MIN_X)/(MAX_X-MIN_X);
  int cy = display_height*(MAX_Y-(double)y)/(MAX_Y-MIN_Y);
 
  mouseMove(display,cx,cy);
  
}

void mouseMove(Display *display,int x,int y){
  XWarpPointer(display,None,RootWindow(display,0),0,0,0,0,x,y);
}
void mouseClick(int button) {
  Display *display = XOpenDisplay(NULL);
  XEvent event;
  if(display == NULL) {
    fprintf(stderr, "Errore nell'apertura del Display !!!\n");
    exit(EXIT_FAILURE);
  }
  memset(&event, 0x00, sizeof(event));
  event.type = ButtonPress;
  event.xbutton.button = button;
  event.xbutton.same_screen = True;
  
  XQueryPointer(display, RootWindow(display, DefaultScreen(display)),
    &event.xbutton.root, &event.xbutton.window, 
    &event.xbutton.x_root, &event.xbutton.y_root, 
    &event.xbutton.x, &event.xbutton.y, &event.xbutton.state);
  
  event.xbutton.subwindow = event.xbutton.window;
  
  while(event.xbutton.subwindow) {
    event.xbutton.window = event.xbutton.subwindow;
    XQueryPointer(display, event.xbutton.window, 
      &event.xbutton.root, &event.xbutton.subwindow, 
      &event.xbutton.x_root, &event.xbutton.y_root, 
      &event.xbutton.x, &event.xbutton.y, &event.xbutton.state
    );
  }
  if(XSendEvent(display,PointerWindow,True,0xfff,&event)==0) 
    fprintf(stderr, "Error\n");
  XFlush(display);
  usleep(100000);
  event.type = ButtonRelease;
  event.xbutton.state = 0x100;
  if(XSendEvent(display,PointerWindow,True,0xfff,&event)==0) 
    fprintf(stderr, "Error\n");
  XFlush(display);
  XCloseDisplay(display);
}
