#include "TouchPlus.h"

using namespace std;


// evil global variables I would like to get rid of
int decompressJPEG = 0;
uvc_frame_t *bgr;
unsigned char * liveData;
volatile long currentFrame = 0;

uvc_context_t * ctx = NULL;
uvc_device_handle_t* devh = NULL;
uvc_device_t *dev;
libusb_device_handle * dev_handle;

long getCurrentFrame()
{
    return currentFrame;
}

unsigned char * getDataPointer(){
    return liveData;
}
//////////////// RECEIVE PACKETS ////////////////////////

void cb(uvc_frame_t *frame, void *ptr) {
  static int counter = 0;
 
  uvc_error_t ret;
  /* Do the BGR conversion */
  if (decompressJPEG==0)
    ret = uvc_any2bgr(frame, bgr);
  else {
    ret = uvc_mjpeg2rgb(frame, bgr);
    if (ret) {
      uvc_perror(ret, "change to BGR error");
      uvc_free_frame(bgr);
      return;
    }
    unsigned char tmp;
    //swap red and blue
      memcpy(liveData,bgr->data,frame->width*frame->height*3);
    
    for (int i =0;i< frame->width*frame->height*3;i+=3){
      tmp =liveData[i+2];
      liveData[i+2]=liveData[i];
      liveData[i]= tmp;

    }
    currentFrame ++;
    
  }

}


void init_camera(){
    uvc_error_t res;
    uvc_device_t *dev;
    res = uvc_init(&ctx, NULL);
    if (res < 0) {
       uvc_perror(res, "uvc_init");
     return ;
    }
    puts("UVC initialized");
    res = uvc_find_device(ctx, &dev,0x1e4e, 0x0107, NULL); /* filter devices: vendor_id, product_id, "serial_num" */
    if (res < 0) {
      uvc_perror(res, "uvc_find_device"); /* no devices found */
    } else {
      puts("Device found");
      res = uvc_open(dev, &devh);
      cout<<"devh "<<devh<<endl;
      //cout<<"devh info "<<*devh<<endl;
     // uvc_print_diag(devh, stderr);
    }
    liveData = (unsigned char*)malloc(1280*480*3);
    fprintf(stderr,"returning libusb handle\n");
    dev_handle = uvc_get_libusb_handle(devh);
}


void printerror(int r){
  if (r==LIBUSB_ERROR_TIMEOUT){
    fprintf(stderr,"TIME-OUT occured\n");
  }
  else if (r==LIBUSB_ERROR_PIPE){
    fprintf(stderr,"control request was not supported by the device\n");
  }
  else if (r==LIBUSB_ERROR_NO_DEVICE)
  {
    fprintf(stderr,"the device has been disconnected\n");
  }
}


void read_ADDR_85(libusb_device_handle *dev_handle, unsigned short wValue = 0x0300){
    
    unsigned short wIndex     = 0x0400;
    unsigned char bmRequestType_get = 0xa1;
    unsigned char data[2];
    int r= libusb_control_transfer(dev_handle,bmRequestType_get,0x85,wValue,wIndex,data,2,10000 );
    if (VERBOSE){
      cout <<"get (0x85) (0x"<<hex<<wValue<<")returned "<<r<<endl;
      for (int i = 0;i<2; i++){
          printf("data[%d] = %x\n",i,data[i]);
      } 
    }
    

}
unsigned char read_ADDR_81(libusb_device_handle *dev_handle, unsigned short wValue= 0x0300, int length = 4){

    unsigned short wIndex     = 0x0400;
    unsigned char bmRequestType_get = 0xa1;
    unsigned char data[length];
    int r= libusb_control_transfer(dev_handle,bmRequestType_get,0x81,wValue,wIndex,data,length,10000 );
    if (VERBOSE){
      cout <<"get (0x81) returned "<<r<<endl;
      for (int i = 0;i<length; i++){
          printf("data[%d] = %x\n",i,data[i]);
      } 
    }
    return data[1];
}
unsigned char read_ADDR_81(libusb_device_handle *dev_handle,unsigned char * data, unsigned short wValue= 0x0300, int length = 4){

    unsigned short wIndex     = 0x0400;
    unsigned char bmRequestType_get = 0xa1;
    int r= libusb_control_transfer(dev_handle,bmRequestType_get,0x81,wValue,wIndex,data,length,10000 );
    if (VERBOSE){
      cout <<"get (0x81) returned "<<r<<endl;
      for (int i = 0;i<length; i++){
          printf("data[%d] = %x\n",i,data[i]);
      } 
    }
    return 0;
}


void write_ADDR_01(libusb_device_handle *dev_handle, unsigned char * data,unsigned short wValue = 0x0300, int length=4)
{
    unsigned short wIndex     = 0x0400;
    unsigned char bmRequestType_set = 0x21;
    int r= libusb_control_transfer(dev_handle,bmRequestType_set,0x01,wValue,wIndex,data,length,10000 );
    if (VERBOSE){
      cout <<"set (0x01) returned "<<r<<endl;
      for (int i = 0;i<length; i++){
          printf("data[%d] = %x\n",i,data[i]);
      } 
    }
    
}






int startVideoStream(int width, int height, int framerate, int format){
    
  int r;
  //get video stream begin
  uvc_stream_ctrl_t ctrl;
  uvc_error_t res;
  fprintf(stderr,"start streaming!!!\n");



  if (format == UNCOMPRESSED){
    res = uvc_get_stream_ctrl_format_size(
        devh, &ctrl, /* result stored in ctrl */
        UVC_FRAME_FORMAT_YUYV, /* YUV 422, aka YUV 4:2:2. try _COMPRESSED */
        width, height, framerate /* width, height, fps */
    );
  }
  else if (format == MJPEG){
    decompressJPEG = 1;
    res = uvc_get_stream_ctrl_format_size(
        devh, &ctrl, /* result stored in ctrl */
        UVC_FRAME_FORMAT_MJPEG, /* YUV 422, aka YUV 4:2:2. try _COMPRESSED */
        width, height, framerate /* width, height, fps */
    );
  }


  /* Print out the result */
  uvc_print_stream_ctrl(&ctrl, stderr);
  if (res < 0) {
        uvc_perror(res, "get_mode"); /* device doesn't provide a matching stream */
  } 
  else {
    int value = 0;
    bgr = uvc_allocate_frame(width*height*3);
    if (!bgr) {
          printf("unable to allocate bgr frame!");
          return -1;
    }
 //  inputImage.data = (unsigned char*)bgr->data;
    res = uvc_start_streaming(devh, &ctrl, cb, (void*)(&value), 0);
    if (res < 0) {
          uvc_perror(res, "start_streaming"); /* unable to start stream */
    } 
    else {
          puts("Streaming...");
    }
  }
  return 0;
}

int stopVideoStream(){
    uvc_stop_streaming(devh);
    uvc_free_frame(bgr);
    uvc_close(devh);
   // uvc_unref_device(dev);
    uvc_exit(ctx);
    return 0;
}

int writeFlash(unsigned char * data, int length){
  
  
    unsigned char count[1];
    read_ADDR_81(dev_handle,count,0x0a00,1);

    read_ADDR_85(dev_handle,0x0b00);  
    unsigned char mystery[16]= {count[0],0x81,0x05,0x00,0x00,0xc8,0x00,0x00,0x00,(unsigned char)length,0x00,0x00,0x00,0x00,0x00,0x00};
    unsigned char mystery2[16]={count[0],0x02,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
    write_ADDR_01(dev_handle,mystery,0x0b00,16);

    read_ADDR_85(dev_handle,0x0c00);
    write_ADDR_01(dev_handle,data,0x0c00,length);

    read_ADDR_85(dev_handle,0x0b00); 
    write_ADDR_01(dev_handle,mystery2,0x0b00,16);

    read_ADDR_85(dev_handle,0x0a00);
  
    write_ADDR_01(dev_handle,count,0x0a00,1);
    //closeTouchPlusDevHandle(dev_handle);
    return 0;
}


int readFlash(unsigned char * data, int length){
  
    read_ADDR_85(dev_handle,0x0a00); 
    unsigned char count[1]; 
    read_ADDR_81(dev_handle,count,0x0a00,1);
    read_ADDR_85(dev_handle,0x0b00);  
    unsigned char mystery[16]= {count[0],0x41,0x05,0x00,0x00,0xc8,0x00,0x00,0x00,(unsigned char)length,0x00,0x00,0x00,0x00,0x00,0x00};
    write_ADDR_01(dev_handle,mystery,0x0b00,16);

    read_ADDR_85(dev_handle,0x0c00);
    read_ADDR_81(dev_handle,data,0x0c00,length);

    
    write_ADDR_01(dev_handle,count,0x0a00,1);
   // closeTouchPlusDevHandle(dev_handle);
    return 0;
}



int getAccelerometerValues(int *x, int *y, int *z)
{

    read_ADDR_85(dev_handle);  
    unsigned char data[4];
    data[0] = 0xa0;
    data[1] = 0x80;
    data[2] = 0x00;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    unsigned char acc_x_1 =read_ADDR_81(dev_handle); 
   // printf("val 0 = %u\n", acc_x_1);

    read_ADDR_85(dev_handle);  
    data[0] = 0xa0;
    data[1] = 0x81;
    data[2] = 0x00;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    unsigned char acc_x_2 =read_ADDR_81(dev_handle); 
  //  printf("val 1 = %u\n", acc_x_2);

    read_ADDR_85(dev_handle);  
    data[0] = 0xa0;
    data[1] = 0x82;
    data[2] = 0x00;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    unsigned char acc_y_1 =read_ADDR_81(dev_handle); 
  //  printf("val 2 = %u\n", acc_y_1);

    read_ADDR_85(dev_handle);  
    data[0] = 0xa0;
    data[1] = 0x83;
    data[2] = 0x00;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    unsigned char acc_y_2=read_ADDR_81(dev_handle); 
    //printf("val 3 = %u\n", acc_y_2);
 
    read_ADDR_85(dev_handle);  
    data[0] = 0xa0;
    data[1] = 0x84;
    data[2] = 0x00;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    unsigned char acc_z_1 =read_ADDR_81(dev_handle); 
    //printf("val 4 = %u\n", acc_z_1);

    read_ADDR_85(dev_handle);  
    data[0] = 0xa0;
    data[1] = 0x85;
    data[2] = 0x00;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    unsigned char acc_z_2 =read_ADDR_81(dev_handle); 
    //printf("val 6 = %u\n", acc_z_2);

    *x= (int)acc_x_1;
    *y= (int)acc_y_1;
    *z= (int)acc_z_1;

    return 0;
}

float getGlobalGain(){
 
  read_ADDR_85(dev_handle,0x0200);  
  unsigned char data[6];
    data[0] = 0x99;
    data[1] = 0x42;
    data[2] = 0x00;
    data[3] = 0x00;
    data[4] = 0x00;
    data[5] = 0x00;
    write_ADDR_01(dev_handle,data,0x0200,6);
    read_ADDR_85(dev_handle,0x0200);
    unsigned char gain =read_ADDR_81(dev_handle,0x0200,6);
    printf("gain = %x",gain);
    float toReturn = (float)gain;
    return toReturn/7.75;
}

int setGlobalGain(float ggain){

    read_ADDR_85(dev_handle,0x0200);  
    unsigned char data[6];
    int toSet = (int)(ggain*7.75);
    if (toSet > 255 )toSet = 255;
    data[0] = 0x19;
    data[1] = 0x42;
    data[2] = 0x00;
    data[3] = toSet;
    data[4] = 0x00;
    data[5] = 0x00;
    write_ADDR_01(dev_handle,data,0x0200,6);
    read_ADDR_85(dev_handle,0x0200);
    unsigned char gain =read_ADDR_81(dev_handle,0x0200,6);
  
    return 0;
}

int setExposureTime(float time){
    //convert to the time to integer
    unsigned short mytime = (unsigned short)(time*90.08);

    unsigned char data[6];

    read_ADDR_85(dev_handle);  
    data[0] = 0x20;
    data[1] = 0xa0;
    data[2] = 0x00;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle);

    read_ADDR_85(dev_handle);  
    data[0] = 0x20;
    data[1] = 0xa1;
    data[2] = 0x3b;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle);

    read_ADDR_85(dev_handle);  
    data[0] = 0xa0;
    data[1] = 0xa2;
    data[2] = 0x00;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle);

    read_ADDR_85(dev_handle);  
    data[0] = 0xa0;
    data[1] = 0xa3;
    data[2] = 0x00;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle);

    read_ADDR_85(dev_handle);  
    data[0] = 0xa0;
    data[1] = 0xa4;
    data[2] = 0x00;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle);

  
    read_ADDR_85(dev_handle);  
    data[0] = 0xa0;
    data[1] = 0xa5;
    data[2] = 0x00;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle);


    read_ADDR_85(dev_handle);  
    data[0] = 0x20;
    data[1] = 0xa0;
    data[2] = 0x00;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle);

    read_ADDR_85(dev_handle);  
    data[0] = 0x20;
    data[1] = 0xa1;
    data[2] = 0xe0;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle);


    read_ADDR_85(dev_handle);  
    data[0] = 0xa0;
    data[1] = 0xa2;
    data[2] = 0x00;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle);

 
    read_ADDR_85(dev_handle);  
    data[0] = 0xa0;
    data[1] = 0xa2;
    data[2] = 0x00;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle);


    read_ADDR_85(dev_handle);  
    data[0] = 0xa0;
    data[1] = 0xa3;
    data[2] = 0x00;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle);


    read_ADDR_85(dev_handle);  
    data[0] = 0xa0;
    data[1] = 0xa4;
    data[2] = 0x00;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle);


    read_ADDR_85(dev_handle);  
    data[0] = 0xa0;
    data[1] = 0xa5;
    data[2] = 0x00;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle);

   
    read_ADDR_85(dev_handle);  
    data[0] = 0x20;
    data[1] = 0xa0;
    data[2] = 0x00;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle);

   
    read_ADDR_85(dev_handle);  
    data[0] = 0x20;
    data[1] = 0xa1;
    data[2] = 0xe0;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle);


    read_ADDR_85(dev_handle);  
    data[0] = 0xa0;
    data[1] = 0xa2;
    data[2] = 0xe0;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle);

 
    read_ADDR_85(dev_handle);  
    data[0] = 0xa0;
    data[1] = 0xa3;
    data[2] = 0x00;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle);

    read_ADDR_85(dev_handle);  
    data[0] = 0x20;
    data[1] = 0xa0;
    data[2] = 0x00;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle);

  
    read_ADDR_85(dev_handle);  
    data[0] = 0x20;
    data[1] = 0xa1;
    data[2] = 0x33;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle);


    read_ADDR_85(dev_handle);  
    data[0] = 0xa0;
    data[1] = 0xa2;
    data[2] = 0x00;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle);

   
    read_ADDR_85(dev_handle);  
    data[0] = 0xa0;
    data[1] = 0xa3;
    data[2] = 0x00;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle);


    read_ADDR_85(dev_handle);  
    data[0] = 0x20;
    data[1] = 0xa0;
    data[2] = 0x00;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle);

     
    read_ADDR_85(dev_handle);  
    data[0] = 0x20;
    data[1] = 0xa1;
    data[2] = 0x37;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle);


    read_ADDR_85(dev_handle);  
    data[0] = 0xa0;
    data[1] = 0xa2;
    data[2] = 0x00;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle);

 
    read_ADDR_85(dev_handle);  
    data[0] = 0xa0;
    data[1] = 0xa3;
    data[2] = 0x00;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle);

    read_ADDR_85(dev_handle);  
    data[0] = 0xa0;
    data[1] = 0xa4;
    data[2] = 0x00;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle);

  
    read_ADDR_85(dev_handle);  
    data[0] = 0xa0;
    data[1] = 0xa5;
    data[2] = 0x00;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle);


    unsigned char byte0 = (unsigned char)(mytime&0x00ff);
    unsigned char byte1 = (unsigned char)(mytime>>8);
   
    read_ADDR_85(dev_handle,0x0200);  
    data[0] = 0x19;
    data[1] = 0x42;
    data[2] = 0x0f;
    data[3] = byte1;
    data[4] = 0x00;
    data[5] = 0x00;
    write_ADDR_01(dev_handle,data,0x0200,6);
    read_ADDR_85(dev_handle,0x0200);
    read_ADDR_81(dev_handle,0x0200,6);
    

  
    read_ADDR_85(dev_handle,0x0200);  
    data[0] = 0x19;
    data[1] = 0x42;
    data[2] = 0x10;
    data[3] = byte0;
    data[4] = 0x00;
    data[5] = 0x00;
    write_ADDR_01(dev_handle,data,0x0200,6);
    read_ADDR_85(dev_handle,0x0200);
    read_ADDR_81(dev_handle,0x0200,6);

  
    read_ADDR_85(dev_handle,0x0200);  
    data[0] = 0x19;
    data[1] = 0x42;
    data[2] = 0x2e;
    data[3] = 0x00;
    data[4] = 0x00;
    data[5] = 0x00;
    write_ADDR_01(dev_handle,data,0x0200,6);
    read_ADDR_85(dev_handle,0x0200);
    read_ADDR_81(dev_handle,0x0200,6);

 
    read_ADDR_85(dev_handle,0x0200);  
    data[0] = 0x19;
    data[1] = 0x42;
    data[2] = 0x2d;
    data[3] = 0x00;
    data[4] = 0x00;
    data[5] = 0x00;
    write_ADDR_01(dev_handle,data,0x0200,6);
    read_ADDR_85(dev_handle,0x0200);
    read_ADDR_81(dev_handle,0x0200,6);

    return 0;
}

float getExposureTime(){

    unsigned char data[6];

    read_ADDR_85(dev_handle);  
    data[0] = 0x20;
    data[1] = 0xa0;
    data[2] = 0x00;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle);

    read_ADDR_85(dev_handle);  
    data[0] = 0x20;
    data[1] = 0xa1;
    data[2] = 0x3b;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle);


    read_ADDR_85(dev_handle);  
    data[0] = 0xa0;
    data[1] = 0xa2;
    data[2] = 0x00;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle);


    read_ADDR_85(dev_handle);
    data[0] = 0xa0;
    data[1] = 0xa3;
    data[2] = 0x00;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle);

   
    read_ADDR_85(dev_handle);
    data[0] = 0xa0;
    data[1] = 0xa4;
    data[2] = 0x00;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle);

   
    read_ADDR_85(dev_handle);
    data[0] = 0xa0;
    data[1] = 0xa5;
    data[2] = 0x00;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle);

   
    read_ADDR_85(dev_handle);
    data[0] = 0x20;
    data[1] = 0xa0;
    data[2] = 0x00;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle);


    read_ADDR_85(dev_handle);
    data[0] = 0x20;
    data[1] = 0xa1;
    data[2] = 0xe0;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle);

 
    read_ADDR_85(dev_handle);
    data[0] = 0xa0;
    data[1] = 0xa2;
    data[2] = 0x00;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle);


    read_ADDR_85(dev_handle);
    data[0] = 0xa0;
    data[1] = 0xa3;
    data[2] = 0x00;
    data[3] = 0x00;
    write_ADDR_01(dev_handle,data);
    read_ADDR_85(dev_handle);
    read_ADDR_81(dev_handle);

    read_ADDR_85(dev_handle,0x0200);
    data[0] = 0x99;
    data[1] = 0x42;
    data[2] = 0x0f;
    data[3] = 0x00;
    data[4] = 0x00;
    data[5] = 0x00;
   
 
    write_ADDR_01(dev_handle,data,0x0200,6);
    read_ADDR_85(dev_handle,0x0200);
    unsigned short temp1 =(unsigned short)read_ADDR_81(dev_handle,0x0200,6);

  
    read_ADDR_85(dev_handle,0x0200);
    data[0] = 0x99;
    data[1] = 0x42;
    data[2] = 0x10;
    data[3] = 0x00;
    data[4] = 0x00;
    data[5] = 0x00;

  
    write_ADDR_01(dev_handle,data,0x0200);
    read_ADDR_85(dev_handle,0x0200);
    unsigned short temp2 = (unsigned short)read_ADDR_81(dev_handle,0x0200,6);

    unsigned short tempdata = 0;
    tempdata +=temp1;
    tempdata = tempdata << 8;
    tempdata += temp2;
    float toReturn = (float)tempdata;
    toReturn /=90.08;
    return toReturn;
}





int disable_ae(){

      unsigned char endpoint = 0x83;
      unsigned char bmRequestType_set = 0x21;
      unsigned char bmRequestType_get = 0xa1;
      unsigned char GET_CUR = 0x85;
      unsigned char SET_CUR = 0x01;
      unsigned short wValue =0x0300;
      unsigned short wIndex = 0x0400;
      unsigned char data[5];
      data[0] = 0x00;
      data[1] = 0x00;
      data[2] = 0x00;
      data[3] = 0x00;
       
      int r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,2,10000 );
      cout <<"get returned "<<r<<endl;
      for (int i = 0;i<2; i++){
        printf("data[%d] = %x\n",i,data[i]);
      } 
      data[0] = 0x20;
      data[1] = 0xa0;
      data[2] = 0x00;
      data[3] = 0x00;
      r= libusb_control_transfer(dev_handle,bmRequestType_set,SET_CUR,wValue,wIndex,data,4,10000 );
      cout <<"set returned "<<r<<endl;
      for (int i = 0;i<4; i++){
        printf("data[%d] = %x\n",i,data[i]);
      } 

      r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,2,10000 );
      cout <<"get returned "<<r<<endl;
      for (int i = 0;i<2; i++){
        printf("data[%d] = %x\n",i,data[i]);
      } 
      GET_CUR = 0x81;
      r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,4,10000 );
      cout <<"get returned "<<r<<endl;
      for (int i = 0;i<4; i++){
        printf("data[%d] = %x\n",i,data[i]);
      } 
      GET_CUR = 0x85;
      r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,2,10000 );
      cout <<"get returned "<<r<<endl;
      for (int i = 0;i<2; i++){
        printf("data[%d] = %x\n",i,data[i]);
      } 

      data[0] = 0x20;
      data[1] = 0xa1;
      data[2] = 0x23;
      data[3] = 0x00;
      r= libusb_control_transfer(dev_handle,bmRequestType_set,SET_CUR,wValue,wIndex,data,4,10000 );
      cout <<"set returned "<<r<<endl;
      for (int i = 0;i<4; i++){
        printf("data[%d] = %x\n",i,data[i]);
      } 
      r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,2,10000 );
      cout <<"get returned "<<r<<endl;
      for (int i = 0;i<2; i++){
        printf("data[%d] = %x\n",i,data[i]);
      } 
      GET_CUR = 0x81;
      r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,4,10000 );
      cout <<"get returned "<<r<<endl;
      for (int i = 0;i<4; i++){
        printf("data[%d] = %x\n",i,data[i]);
      } 
      GET_CUR = 0x85;
      r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,2,10000 );
      cout <<"get returned "<<r<<endl;
      for (int i = 0;i<2; i++){
        printf("data[%d] = %x\n",i,data[i]);
      } 
      data[0] = 0x20;
      data[1] = 0xa2;
      data[2] = 0x00;
      data[3] = 0x00;
      r= libusb_control_transfer(dev_handle,bmRequestType_set,SET_CUR,wValue,wIndex,data,4,10000 );
      cout <<"set returned "<<r<<endl;
      for (int i = 0;i<4; i++){
        printf("data[%d] = %x\n",i,data[i]);
      } 
      GET_CUR = 0x85;
      r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,2,10000 );
      cout <<"get returned "<<r<<endl;
      for (int i = 0;i<2; i++){
        printf("data[%d] = %x\n",i,data[i]);
      } 
       GET_CUR = 0x81;
      r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,4,10000 );
      cout <<"get returned "<<r<<endl;
      for (int i = 0;i<4; i++){
        printf("data[%d] = %x\n",i,data[i]);
      } 


    return 0;

}

int enable_ae(){

  //  libusb_device **devs; //pointer to pointer of device, used to retrieve a list of devices
  printf("called\n");
   
    int r;
    /*
    libusb_device_descriptor *   desc;
    libusb_device *  device;   
    libusb_context *ctx = NULL; //a libusb session
    int r; //for return values
    ssize_t cnt; //holding number of devices in list
    r = libusb_init(&ctx); //initialize the library for the session we just declared
        if(r < 0) {
        cout<<"Init Error "<<r<<endl; //there was an error
        return 1;
    }

    dev_handle = libusb_open_device_with_vid_pid(ctx, 0x1e4e, 0x0107); //these are vendorID and productID I found for my usb device
    if(dev_handle == NULL )     
      cout<<"Cannot open device"<<endl;
    else
        cout<<"Device Opened"<<endl;
   // libusb_free_device_list(devs, 1); //free the list, unref the devices in it

    //release the driver from the kernel
    if(libusb_kernel_driver_active(dev_handle, 0) == 1) { //find out if kernel driver is attached
        cout<<"Kernel Driver Active"<<endl;
        if(libusb_detach_kernel_driver(dev_handle, 0) == 0) //detach it
            cout<<"Kernel Driver Detached!"<<endl;
    }
    else{
      printf("kernel driver not active\n");
    }
    
    r = libusb_claim_interface(dev_handle, 0); //claim interface 0 (the first) of device (mine had jsut 1)
    if(r < 0) {
        cout<<"Cannot Claim Interface"<<endl;
        return 1;
    }
    */
    //cout<<"Claimed Interface"<<endl;
   // device = libusb_get_device (dev_handle);
  /*
    int configuration = -1;
    r= libusb_get_configuration(dev_handle,&configuration);

    if (r == 0){
        cout << "GET CONFIGURATION SUCCESS -- Value is "<<configuration<<endl;
    }
    */
    libusb_transfer transfer ={0};
      unsigned char endpoint = 0x83;
      unsigned char bmRequestType_set = 0x21;
      unsigned char bmRequestType_get = 0xa1;
      unsigned char GET_CUR = 0x85;
      unsigned char SET_CUR = 0x01;
      unsigned short wValue =0x0300;
      unsigned short wIndex = 0x0400;
      unsigned char data[5];
      data[0] = 0x00;
      data[1] = 0x00;
      data[2] = 0x00;
      data[3] = 0x00;
       
      r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,2,10000 );
      cout <<"get returned "<<r<<endl;
      for (int i = 0;i<2; i++){
        printf("data[%d] = %x\n",i,data[i]);
      } 
      data[0] = 0x20;
      data[1] = 0xa0;
      data[2] = 0x00;
      data[3] = 0x00;
      r= libusb_control_transfer(dev_handle,bmRequestType_set,SET_CUR,wValue,wIndex,data,4,10000 );
      cout <<"set returned "<<r<<endl;
      for (int i = 0;i<4; i++){
        printf("data[%d] = %x\n",i,data[i]);
      } 

      r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,2,10000 );
      cout <<"get returned "<<r<<endl;
      for (int i = 0;i<2; i++){
        printf("data[%d] = %x\n",i,data[i]);
      } 
      GET_CUR = 0x81;
      r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,4,10000 );
      cout <<"get returned "<<r<<endl;
      for (int i = 0;i<4; i++){
        printf("data[%d] = %x\n",i,data[i]);
      } 
      GET_CUR = 0x85;
      r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,2,10000 );
      cout <<"get returned "<<r<<endl;
      for (int i = 0;i<2; i++){
        printf("data[%d] = %x\n",i,data[i]);
      } 

      data[0] = 0x20;
      data[1] = 0xa1;
      data[2] = 0x23;
      data[3] = 0x00;
      r= libusb_control_transfer(dev_handle,bmRequestType_set,SET_CUR,wValue,wIndex,data,4,10000 );
      cout <<"set returned "<<r<<endl;
      for (int i = 0;i<4; i++){
        printf("data[%d] = %x\n",i,data[i]);
      } 
      r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,2,10000 );
      cout <<"get returned "<<r<<endl;
      for (int i = 0;i<2; i++){
        printf("data[%d] = %x\n",i,data[i]);
      } 
      GET_CUR = 0x81;
      r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,4,10000 );
      cout <<"get returned "<<r<<endl;
      for (int i = 0;i<4; i++){
        printf("data[%d] = %x\n",i,data[i]);
      } 
      GET_CUR = 0x85;
      r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,2,10000 );
      cout <<"get returned "<<r<<endl;
      for (int i = 0;i<2; i++){
        printf("data[%d] = %x\n",i,data[i]);
      } 
      data[0] = 0x20;
      data[1] = 0xa2;
      data[2] = 0x01;
      data[3] = 0x00;
      r= libusb_control_transfer(dev_handle,bmRequestType_set,SET_CUR,wValue,wIndex,data,4,10000 );
      cout <<"set returned "<<r<<endl;
      for (int i = 0;i<4; i++){
        printf("data[%d] = %x\n",i,data[i]);
      } 
      GET_CUR = 0x85;
      r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,2,10000 );
      cout <<"get returned "<<r<<endl;
      for (int i = 0;i<2; i++){
        printf("data[%d] = %x\n",i,data[i]);
      } 
      GET_CUR = 0x81;
      r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,4,10000 );
      cout <<"get returned "<<r<<endl;
      for (int i = 0;i<4; i++){
        printf("data[%d] = %x\n",i,data[i]);
      } 
/*
    r = libusb_release_interface(dev_handle, 0); //release the claimed interface
    if(r!=0) {
        cout<<"Cannot Release Interface"<<endl;
        return 1;
    }
    */
   // libusb_close(dev_handle); //close the device we opened
   // libusb_exit(ctx); //needs to be called to end the
    return 0;
}

int tp_do_software_unlock(){
    libusb_device **devs; //pointer to pointer of device, used to retrieve a list of devices
    libusb_device_handle *dev_handle; //a device handle
    libusb_device_descriptor *   desc;
    libusb_device *  device;   
    libusb_context *ctx = NULL; //a libusb session
    int r; //for return values
    ssize_t cnt; //holding number of devices in list
    r = libusb_init(&ctx); //initialize the library for the session we just declared
        if(r < 0) {
        cout<<"Init Error "<<r<<endl; //there was an error
        return 1;
    }

    dev_handle = libusb_open_device_with_vid_pid(ctx, 0x1e4e, 0x0107); //these are vendorID and productID I found for my usb device
    if(dev_handle == NULL )     
      cout<<"Cannot open device"<<endl;
    else
        cout<<"Device Opened"<<endl;
    //libusb_free_device_list(devs, 1); //free the list, unref the devices in it

    //release the driver from the kernel
    if(libusb_kernel_driver_active(dev_handle, 0) == 1) { //find out if kernel driver is attached
        cout<<"Kernel Driver Active"<<endl;
        if(libusb_detach_kernel_driver(dev_handle, 0) == 0) //detach it
            cout<<"Kernel Driver Detached!"<<endl;
    }
    r = libusb_claim_interface(dev_handle, 0); //claim interface 0 (the first) of device (mine had jsut 1)
    if(r < 0) {
        cout<<"Cannot Claim Interface"<<endl;
        return 1;
    }
    cout<<"Claimed Interface"<<endl;
    device = libusb_get_device (dev_handle);
    int configuration = -1;
    r= libusb_get_configuration(dev_handle,&configuration);

    if (r == 0){
        cout << "GET CONFIGURATION SUCCESS -- Value is "<<configuration<<endl;
    }
    //libusb_transfer transfer ={0};
      unsigned char endpoint = 0x83;
      unsigned char bmRequestType_set = 0x21;
      unsigned char bmRequestType_get = 0xa1;
      unsigned char GET_CUR = 0x85;
      unsigned char SET_CUR = 0x01;
      unsigned short wValue =0x0300;
      unsigned short wIndex = 0x0400;
      unsigned char data[5];
      data[0] = 0x00;
      data[1] = 0x00;
      data[2] = 0x00;
      data[3] = 0x00;
       
      r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,2,10000 );
      data[0]=0x82;
      data[1]=0xf1;
      data[2]=0xf8;
      data[3]=0x00;

      r= libusb_control_transfer(dev_handle,bmRequestType_set,SET_CUR,wValue,wIndex,data,4,10000 );
      data[0]=0x00;
      data[1]=0x00;
      r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,2,10000 );

      data[0]=0x00;
      data[1]=0x00;
      data[2]=0x00;
      data[3]=0x00;
      GET_CUR = 0x81;
      r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,4,10000 );

      data[0]=0x00;
      data[1]=0x00;
      GET_CUR=0x85;
      r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,2,10000);

      data[0]=0x02;
      data[1]=0xf1;
      data[2]=0xf8;
      data[3]=0x40;
      r= libusb_control_transfer(dev_handle,bmRequestType_set,SET_CUR,wValue,wIndex,data,4,0 );

      data[0]=0x00;
      data[1]=0x00;
      r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,2,0 );

      data[0]=0x00;
      data[1]=0x00;
      data[2]=0x00;
      data[3]=0x00;
      GET_CUR=0x81;
      r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,4,0 );
      fprintf(stderr,"Camera unlocked\n");
    r = libusb_release_interface(dev_handle, 0); //release the claimed interface
    if(r!=0) {
        cout<<"Cannot Release Interface"<<endl;
        return 1;
    }
    libusb_close(dev_handle); //close the device we opened
    libusb_exit(ctx); //needs to be called to end the
    return 0;
}


int enable_awb(){
   
      unsigned char endpoint = 0x83;
      unsigned char bmRequestType_set = 0x21;
      unsigned char bmRequestType_get = 0xa1;
      unsigned char GET_CUR = 0x85;
      unsigned char SET_CUR = 0x01;
      unsigned short wValue =0x0300;
      unsigned short wIndex = 0x0400;
      unsigned char data[5];
      data[0] = 0x00;
      data[1] = 0x00;
      data[2] = 0x00;
      data[3] = 0x00;
       
      int r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,2,10000 );
      cout <<"get returned "<<r<<endl;
      for (int i = 0;i<2; i++){
        printf("data[%d] = %x\n",i,data[i]);
      } 
      data[0] = 0x20;
      data[1] = 0xa8;
      data[2] = 0x00;
      data[3] = 0x00;
      r= libusb_control_transfer(dev_handle,bmRequestType_set,SET_CUR,wValue,wIndex,data,4,10000 );
      cout <<"set returned "<<r<<endl;
      for (int i = 0;i<4; i++){
        printf("data[%d] = %x\n",i,data[i]);
      } 

      r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,2,10000 );
      cout <<"get returned "<<r<<endl;
      for (int i = 0;i<2; i++){
        printf("data[%d] = %x\n",i,data[i]);
      } 
      GET_CUR = 0x81;
      r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,4,10000 );
      cout <<"get returned "<<r<<endl;
      for (int i = 0;i<4; i++){
        printf("data[%d] = %x\n",i,data[i]);
      } 
      GET_CUR = 0x85;
      r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,2,10000 );
      cout <<"get returned "<<r<<endl;
      for (int i = 0;i<2; i++){
        printf("data[%d] = %x\n",i,data[i]);
      } 

      data[0] = 0x20;
      data[1] = 0xa9;
      data[2] = 0x0C;
      data[3] = 0x00;
      r= libusb_control_transfer(dev_handle,bmRequestType_set,SET_CUR,wValue,wIndex,data,4,10000 );
      cout <<"set returned "<<r<<endl;
      for (int i = 0;i<4; i++){
        printf("data[%d] = %x\n",i,data[i]);
      } 
      r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,2,10000 );
      cout <<"get returned "<<r<<endl;
      for (int i = 0;i<2; i++){
        printf("data[%d] = %x\n",i,data[i]);
      } 
      GET_CUR = 0x81;
      r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,4,10000 );
      cout <<"get returned "<<r<<endl;
      for (int i = 0;i<4; i++){
        printf("data[%d] = %x\n",i,data[i]);
      } 
      GET_CUR = 0x85;
      r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,2,10000 );
      cout <<"get returned "<<r<<endl;
      for (int i = 0;i<2; i++){
        printf("data[%d] = %x\n",i,data[i]);
      } 
      data[0] = 0x20;
      data[1] = 0xaa;
      data[2] = 0x01;
      data[3] = 0x00;
      r= libusb_control_transfer(dev_handle,bmRequestType_set,SET_CUR,wValue,wIndex,data,4,10000 );
      cout <<"set returned "<<r<<endl;
      for (int i = 0;i<4; i++){
        printf("data[%d] = %x\n",i,data[i]);
      } 
      GET_CUR = 0x85;
      r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,2,10000 );
      cout <<"get returned "<<r<<endl;
      for (int i = 0;i<2; i++){
        printf("data[%d] = %x\n",i,data[i]);
      } 
       GET_CUR = 0x81;
      r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,4,10000 );
      cout <<"get returned "<<r<<endl;
      for (int i = 0;i<4; i++){
        printf("data[%d] = %x\n",i,data[i]);
      } 

    return 0;

}

int disable_awb(){
  
      unsigned char endpoint = 0x83;
      unsigned char bmRequestType_set = 0x21;
      unsigned char bmRequestType_get = 0xa1;
      unsigned char GET_CUR = 0x85;
      unsigned char SET_CUR = 0x01;
      unsigned short wValue =0x0300;
      unsigned short wIndex = 0x0400;
      unsigned char data[5];
      data[0] = 0x00;
      data[1] = 0x00;
      data[2] = 0x00;
      data[3] = 0x00;
       
      int r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,2,10000 );
      cout <<"get returned "<<r<<endl;
      for (int i = 0;i<2; i++){
        printf("data[%d] = %x\n",i,data[i]);
      } 
      data[0] = 0x20;
      data[1] = 0xa8;
      data[2] = 0x00;
      data[3] = 0x00;
      r= libusb_control_transfer(dev_handle,bmRequestType_set,SET_CUR,wValue,wIndex,data,4,10000 );
      cout <<"set returned "<<r<<endl;
      for (int i = 0;i<4; i++){
        printf("data[%d] = %x\n",i,data[i]);
      } 

      r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,2,10000 );
      cout <<"get returned "<<r<<endl;
      for (int i = 0;i<2; i++){
        printf("data[%d] = %x\n",i,data[i]);
      } 
      GET_CUR = 0x81;
      r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,4,10000 );
      cout <<"get returned "<<r<<endl;
      for (int i = 0;i<4; i++){
        printf("data[%d] = %x\n",i,data[i]);
      } 
      GET_CUR = 0x85;
      r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,2,10000 );
      cout <<"get returned "<<r<<endl;
      for (int i = 0;i<2; i++){
        printf("data[%d] = %x\n",i,data[i]);
      } 

      data[0] = 0x20;
      data[1] = 0xa9;
      data[2] = 0x0C;
      data[3] = 0x00;
      r= libusb_control_transfer(dev_handle,bmRequestType_set,SET_CUR,wValue,wIndex,data,4,10000 );
      cout <<"set returned "<<r<<endl;
      for (int i = 0;i<4; i++){
        printf("data[%d] = %x\n",i,data[i]);
      } 
      r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,2,10000 );
      cout <<"get returned "<<r<<endl;
      for (int i = 0;i<2; i++){
        printf("data[%d] = %x\n",i,data[i]);
      } 
      GET_CUR = 0x81;
      r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,4,10000 );
      cout <<"get returned "<<r<<endl;
      for (int i = 0;i<4; i++){
        printf("data[%d] = %x\n",i,data[i]);
      } 
      GET_CUR = 0x85;
      r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,2,10000 );
      cout <<"get returned "<<r<<endl;
      for (int i = 0;i<2; i++){
        printf("data[%d] = %x\n",i,data[i]);
      } 
      data[0] = 0x20;
      data[1] = 0xaa;
      data[2] = 0x00;
      data[3] = 0x00;
      r= libusb_control_transfer(dev_handle,bmRequestType_set,SET_CUR,wValue,wIndex,data,4,10000 );
      cout <<"set returned "<<r<<endl;
      for (int i = 0;i<4; i++){
        printf("data[%d] = %x\n",i,data[i]);
      } 
      GET_CUR = 0x85;
      r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,2,10000 );
      cout <<"get returned "<<r<<endl;
      for (int i = 0;i<2; i++){
        printf("data[%d] = %x\n",i,data[i]);
      } 
       GET_CUR = 0x81;
      r= libusb_control_transfer(dev_handle,bmRequestType_get,GET_CUR,wValue,wIndex,data,4,10000 );
      cout <<"get returned "<<r<<endl;
      for (int i = 0;i<4; i++){
        printf("data[%d] = %x\n",i,data[i]);
      } 


    return 0;

}
