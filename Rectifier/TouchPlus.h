#ifndef _TOUCHPLUS_
#define _TOUCHPLUS_
#include <iostream>
#include <string>
#include <stdio.h>
#include <unistd.h>
#include <libusb-1.0/libusb.h>
#include <libuvc/libuvc.h>

#define VERBOSE 1
#define MJPEG 1
#define UNCOMPRESSED 0


void	init_camera();

int 	tp_do_software_unlock();
int 	tp_do_software_lock();

int 	enable_ae();
int 	disable_ae();
int 	enable_awb();
int 	disable_awb();

float 	getExposureTime();
int 	setExposureTime(float etime);
float 	getGlobalGain();
int 	setGlobalGain(float ggain);
int 	getAccelerometerValues( int *x, int *y, int *z);
int 	readFlash(unsigned char * data, int length);
int 	writeFlash(unsigned char * data, int length);
int 	startVideoStream(int width, int height, int framerate, int format);
int 	stopVideoStream();
long 	getCurrentFrame();
unsigned char * getDataPointer();


#endif