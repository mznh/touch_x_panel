#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>
//タッチパネル制御
//黄色スタートで
//touch			x+ y+ x- y-
//GPIO_NUM	 0  1  2  3
#define GPIO_0 0
#define GPIO_1 1
#define GPIO_2 2
#define GPIO_3 3
void getTouch(int* x,int* y);
int formatADCdata(unsigned char raw_data[]);

int main(){
	int x,y; 
  if(wiringPiSetup() == -1){ perror("Setup Error"); return 1; }
  wiringPiSPISetup(0,500000);
	pinMode(GPIO_0,OUTPUT); pinMode(GPIO_1,OUTPUT);
	pinMode(GPIO_2,OUTPUT); pinMode(GPIO_3,OUTPUT);

  while (1) {
		getTouch(&x,&y);
		printf("%5d %5d\n",x,y);
  }
  return 0;
}
void getTouch(int* x,int* y){
  unsigned char buf[3];
	pinMode(GPIO_0,OUTPUT); pinMode(GPIO_1,INPUT); 
	pinMode(GPIO_2,OUTPUT); pinMode(GPIO_3,INPUT);
	digitalWrite(GPIO_0,1); digitalWrite(GPIO_2,0);
	usleep(10000);
	buf[0]=0x06;buf[1]=0x40;buf[2]=0x00;
	wiringPiSPIDataRW(0,buf,3);
	*x=formatADCdata(buf);
	digitalWrite(GPIO_0,0);
	usleep(10000);
	//y
	pinMode(GPIO_0,INPUT); pinMode(GPIO_1,OUTPUT); 
	pinMode(GPIO_2,INPUT); pinMode(GPIO_3,OUTPUT);
	digitalWrite(GPIO_1,1); digitalWrite(GPIO_3,0);
	usleep(10000);
	buf[0]=0x06;buf[1]=0x00;buf[2]=0x00;
	wiringPiSPIDataRW(0,buf,3);
	*y=formatADCdata(buf);
	digitalWrite(GPIO_1,0);
	usleep(10000);
  return;
}
int formatADCdata(unsigned char raw_data[]){
	int value=0;	
	value=(raw_data[1]&0x0f)<< 8;
	value=raw_data[2]+value;
	value>>=2;
	return value;
}
