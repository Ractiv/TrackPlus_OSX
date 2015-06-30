#include "Camera.h"

#include "upload.h"
#include <fstream>
#include <string>
#include <iostream>
#include <algorithm>
#include "base64.h"
#include "TouchPlus.h"
#define TP_CAMERA_VID   "1e4e"
#define TP_CAMERA_PID   "0107"
#define USE_DIRECT_SHOW 0
#define BYTE unsigned char *

bool Camera::ready = false;
bool Camera::opened = false;

function<void (Mat& image_in)> Camera::callback;

//static RactivJPEGDecompressor * decomp = new RactivJPEGDecompressor();

unsigned char * myBuffer;
Mat image_out = Mat(480, 1280, CV_8UC3);
static char  fname[256];
long long s_upload_file_size = 0;

bool ZipFile(const std::vector<std::string>& sourceFiles, char * destZip);

void * pHandle = NULL;

Camera::Camera(){}

Camera::Camera(bool _useMJPEG, int _width, int _height, function<void (Mat& image_in)> callback_in)
{
	height	= _height;
	width	= _width;
	useMJPEG = _useMJPEG;
	callback = callback_in;
	doSetup(1);
}

Camera::~Camera()
{
#ifdef _WIN32
	if (ds_camera_)
	{
		ds_camera_->CloseCamera();
		ds_camera_ = NULL;
	}
	free(&buffer);
	free(&bufferRGB);
	free(&depth);
#elif __APPLE__
    
#endif
}


static void frameCallback(BYTE * pBuffer, long lBufferSize)
{
#ifdef _WIN32
	decomp->decompress(pBuffer, lBufferSize, myBuffer, 1280, 480);
	Camera::ready = true;
	Camera::callback(image_out);
#elif __APPLE__
#endif
}

string Camera::getSerialNumber()
{
    string result = "";
    unsigned char serialNumber[10];
#ifdef _WIN32
	eSPAEAWB_ReadFlash(serialNumber, 10);
#elif __APPLE__
    readFlash(serialNumber, 10);
#endif
	for (int i = 0; i < 10; i++) {
		int digit = serialNumber[i];
		result += to_string(digit);
	}
	return result;

}

int Camera::startRecording(char * name)
{
    //Corey come back and get recording working again
	Size S = Size(640, 240);
#ifdef _WIN32
	outputVideo.open(name, CV_FOURCC('M','S','V','C'), 15, S, true);
    strcpy(fname, name);
    
    if (!outputVideo.isOpened())
    {
        cout << "Could not open the output video  " << endl;
        return -1;
    }
    record = true;
    return 0;
#elif __APPLE__
    //startVideoStream(S.width, S.height, 15, MJPEG);
    return 1;
#endif

}

int Camera::stopRecording()
{
	record = false;
	outputVideo.release();
	char commandStr[256];

	// add the names of the source files to be zipped to sourceFiles
	std::vector<std::string>  sourceFiles;
	sourceFiles.push_back(fname);
	char zipped_file[256];
	sprintf(zipped_file, "%s.zip", fname);

	printf("String zip name = %s\n", zipped_file);

	if (false == ZipFile(sourceFiles, zipped_file))
	{
		printf("invalid input filename, can not zip it!");
		return -1;
	}
	else {
		printf("Zip completed, filename = %s\n",zipped_file);
	}

	//std::ifstream ifs("test.jpg");
	FILE* f = fopen(zipped_file, "rb");

	//std::ifstream ifs("test.jpg");
	//FILE* f = fopen("0101123457-2014107-053151.zip", "rb");
	
	//FILE* f = fopen(fname, "rb");
	// Determine file size
	fseek(f, 0, SEEK_END);
	size_t size = ftell(f);
	s_upload_file_size = size;
	char* where = new char[size];

	rewind(f);
	fread(where, sizeof(char), size, f);
	S3FileManager s3fm("AKIAJBQ2VX7SJ2NTVFGA", "KtGD8P6yqbflzf+TD+AWYkioXe97Ygk46ruqt4Kq");
	

	s3fm.S3UploadFile(where, size, zipped_file);
	return 0;
}

int  Camera::isCameraPresent()
{
#ifdef _WIN32
	int devCount;
	int didInit = EtronDI_Init(&pHandle);
	int devNumber = EtronDI_GetDeviceNumber(pHandle);

	WCHAR name[100];
	if (!EtronDI_FindDevice(pHandle)) {
		fprintf(stderr, "Device not found!\n");
		return 0;
	}
	int code = eSPAEAWB_EnumDevice(&devCount);
	eSPAEAWB_SelectDevice(0);
	eSPAEAWB_SetSensorType(1);
	WCHAR myName[255];
	WCHAR targetName[255] = L"Touch+ Camera";
	int deviceWasSelected = 0;
	for (int i = 0; i < devCount; i++){
		eSPAEAWB_GetDevicename(i, myName, 255);
		wcout << myName << endl;
		myName[14] = 0;
		bool found = true;
		int j;
		for (j = 0; j < 13; j++){
			if (myName[j] != targetName[j]){
				found = false;
			}
		}
		if (found){
			eSPAEAWB_SelectDevice(i);
			eSPAEAWB_SetSensorType(1);
			deviceWasSelected = 1;
			cout << "Touch+ Camera found" << endl;
		}

	}
	if (!deviceWasSelected){
		printf("Did not find a Touch+ Camera\n");
		return 0;
	}
#elif __APPLE__
#endif
	return 1;
}

int Camera::do_software_unlock()
{
#ifdef _WIN32
	int present = isCameraPresent();
	int retVal = eSPAEAWB_SWUnlock(0x0107);
	return present;
#elif __APPLE__
    tp_do_software_lock();
    return 1;
#endif
}


int Camera::doSetup(const int & format)
{
#ifdef _WIN32
	do_software_unlock();
	
	ds_camera_ = new CCameraDS();
	int camera_count = CCameraDS::CameraCount();
	char camera_name[255] = { 0 };
	char camera_vid[10] = { 0 };
	char camera_pid[10] = { 0 };
	int i = 0, touchCameraId = -1;
	
	while (i < camera_count)
	{
		CCameraDS::CameraInfo(i, camera_vid, camera_pid);

		// VID PID is more reasonable
		if (0 == strncmp(camera_vid, TP_CAMERA_VID, 4) &&
			0 == strncmp(camera_pid, TP_CAMERA_PID, 4))
		{
			touchCameraId = i;
			break;
		}
		i++;
	}

	if (-1 == touchCameraId)
	{
		return false;
	}
	const int fmt = 1;
	myBuffer = (unsigned char *)malloc(1280 * 480 * 3);
	image_out.data = myBuffer;
	bool retV = ds_camera_->OpenCamera(touchCameraId, format, 1280, 480, 60, frameCallback);
	printf("camera opened = %d\n", retV);
	Camera::opened = true;
	return retV;
#elif __APPLE__
    init_camera();
    return 1;
#endif
}

unsigned char* Camera::getDataPointer()
{
	return myBuffer;
}

int Camera::setExposureTime(int whichSide, float expTime)
{
#ifdef _WIN32
	int retCode= eSPAEAWB_SetExposureTime(whichSide, expTime);
	return retCode;
#elif __APPLE__
    int retCode = setExposureTime(whichSide, expTime);
    return retCode;
#endif
}

float Camera::getExposureTime(int whichSide)
{
	float eTime = -1.0;
#ifdef _WIN32
	int retCode = eSPAEAWB_GetExposureTime(whichSide, &eTime);
#elif __APPLE__
    eTime = getExposureTime(whichSide);
#endif
	return eTime;
}

int Camera::setGlobalGain(int whichSide, float gain)
{
#ifdef _WIN32
	return eSPAEAWB_SetGlobalGain(whichSide, gain);
#elif __APPLE__
    return setGlobalGain(whichSide, gain);
#endif
}

float Camera::getGlobalGain(int whichSide)
{
	float globalGain = -1.0;
#ifdef _WIN32
	eSPAEAWB_GetGlobalGain(whichSide, &globalGain);
#elif __APPLE__
    globalGain = getGlobalGain(whichSide);
#endif
    return globalGain;
}

int Camera::turnLEDsOn()
{
	BYTE gpio_code;
	int retCode = eSPAEAWB_GetGPIOValue(1, &gpio_code);
    gpio_code |= 0x08;
	retCode = eSPAEAWB_SetGPIOValue(1, gpio_code);
	return retCode;
}

int Camera::turnLEDsOff()
{
	BYTE gpio_code;
	int retCode = eSPAEAWB_GetGPIOValue(1, &gpio_code);
	gpio_code &= 0xf7;
	retCode = eSPAEAWB_SetGPIOValue(1, gpio_code);
	return retCode;
}

int Camera::getAccelerometerValues(int *x, int *y, int *z)
{
	return eSPAEAWB_GetAccMeterValue(x, y, z);
}

int	Camera::setColorGains(int whichSide, float red, float green, float blue)
{
	return eSPAEAWB_SetColorGain(whichSide, red, green, blue);
}

int	Camera::getColorGains(int whichSide, float *red, float *green, float * blue)
{
	return eSPAEAWB_GetColorGain(whichSide, red, green, blue);
}

int Camera::enableAutoExposure(int whichSide)
{
	eSPAEAWB_SelectDevice(whichSide);
	return eSPAEAWB_EnableAE();
}

int Camera::disableAutoExposure(int whichSide)
{
	eSPAEAWB_SelectDevice(whichSide);
	return eSPAEAWB_DisableAE();
}

int Camera::enableAutoWhiteBalance(int whichSide)
{
	eSPAEAWB_SelectDevice(whichSide);
	return eSPAEAWB_EnableAWB();
}

int Camera::disableAutoWhiteBalance(int whichSide)
{
	eSPAEAWB_SelectDevice(whichSide);
	return eSPAEAWB_DisableAWB();
}

void Camera::YUY2_to_RGB24_Microsoft(BYTE *pSrc, BYTE *pDst, int cx, int cy)
{
	int nSrcBPS, nDstBPS, x, y, x2, x3, m;
	int ma0, mb0, m02, m11, m12, m21;
	BYTE *pS0, *pD0;
	int Y, U, V, Y2;
	BYTE R, G, B, R2, G2, B2;

	nSrcBPS = cx * 2;
	nDstBPS = ((cx * 3 + 3) / 4) * 4;

	pS0 = pSrc;
	pD0 = pDst + nDstBPS*(cy - 1);
	for (y = 0; y<cy; y++)
	{
		for (x3 = 0, x2 = 0, x = 0; x<cx; x += 2, x2 += 4, x3 += 6)
		{
			Y = (int)pS0[x2 + 0] - 16;
			Y2 = (int)pS0[x2 + 2] - 16;
			U = (int)pS0[x2 + 1] - 128;
			V = (int)pS0[x2 + 3] - 128;
			//
			ma0 = 298 * Y;
			mb0 = 298 * Y2;
			m02 = 409 * V + 128;
			m11 = -100 * U;
			m12 = -208 * V + 128;
			m21 = 516 * U + 128;
			//
			m = (ma0 + m02) >> 8;
			R = (m<0) ? (0) : (m>255) ? (255) : ((BYTE)m);
			m = (ma0 + m11 + m12) >> 8;
			G = (m<0) ? (0) : (m>255) ? (255) : ((BYTE)m);
			m = (ma0 + m21) >> 8;
			B = (m<0) ? (0) : (m>255) ? (255) : ((BYTE)m);
			//
			m = (mb0 + m02) >> 8;
			R2 = (m<0) ? (0) : (m>255) ? (255) : ((BYTE)m);
			m = (mb0 + m11 + m12) >> 8;
			G2 = (m<0) ? (0) : (m>255) ? (255) : ((BYTE)m);
			m = (mb0 + m21) >> 8;
			B2 = (m<0) ? (0) : (m>255) ? (255) : ((BYTE)m);
			//
			pD0[x3] = B;
			pD0[x3 + 1] = G;
			pD0[x3 + 2] = R;
			pD0[x3 + 3] = B2;
			pD0[x3 + 4] = G2;
			pD0[x3 + 5] = R2;
		}
		pS0 += nSrcBPS;
		pD0 -= nDstBPS;
	}
}