#include "camera_initializer_new.h"

double CameraInitializerNew::exposure_current;

bool CameraInitializerNew::exposure_adjusted;

void CameraInitializerNew::init(Camera* camera)
{
	exposure_current = 1;
	exposure_adjusted = false;

	camera->disableAutoExposure(Camera::both);
	camera->disableAutoWhiteBalance(Camera::both);
	camera->turnLEDsOn();
	camera->setExposureTime(Camera::both, exposure_current);

	preset0(camera);
}

bool CameraInitializerNew::adjust_exposure(Camera* camera, Mat& image_preprocessed, const uchar gray_max)
{
	if (exposure_adjusted)
		return true;

	Mat image_blurred;
	GaussianBlur(image_preprocessed, image_blurred, Size(9, 9), 0, 0);

	exposure_current = (double)100 / gray_max;
	if (exposure_current > 7)
		exposure_current = 7;

	camera->setExposureTime(Camera::both, exposure_current);

	cout << (int)gray_max << " " << exposure_current << endl;

	exposure_adjusted = true;
	return exposure_adjusted;
}

void CameraInitializerNew::preset0(Camera* camera)
{
	camera->setGlobalGain(0, 1.0);
	camera->setGlobalGain(1, 1.0);
	camera->setColorGains(0, 2.0, 1.0, 2.0);
	camera->setColorGains(1, 2.0, 1.0, 2.0);
}