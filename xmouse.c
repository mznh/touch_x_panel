#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <string.h>

//キャリブレーションデータ
//toriaezu
#define MIN_X 88
#define MAX_X 810
#define MIN_Y 190 
#define MAX_Y 720

int display_width,display_height;
void mouseMove(Display *display,int x,int y);
void mouseClick(int button); 

int main(int argc , char* argv[]) {
  Display *display;
  int display_width,display_height;
  int i;
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
  i=0;
  while(1){
    int x=0,y=0;
    i++;
    scanf("%d",&x);
    scanf("%d",&y);
    printf("%d %d\n",x,y);
    mouseMove(display,x,y);
    XFlush(display);
    if(i>10)break;
  }
  //mouseClick(Button1);
  XFlush(display);
  /* ディスプレイ・サーバとの接続を解除 */
  XCloseDisplay(display);
  return 0;
}
///////////////////////////////////////////

void calibratedMouseMove(Display *display,int x,int y){
  int cx = display_width*((double)x-MIN_X)/(MAX_X-MIN_X);
  int cy = display_height*((double)y-MIN_Y)/(MAX_Y-MIN_Y);
 
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
