#pragma once

#include <opencv2/opencv.hpp>
#include "camera.h"
#include "globals.h"

using namespace cv;

class CameraInitializerNew
{
public:
	static const int exposure_min = 0;
	static const int exposure_max = 7;

	static double exposure_current;
	
	static bool exposure_adjusted;

	static void init(Camera* camera);
	static bool adjust_exposure(Camera* camera, Mat& image_preprocessed, const uchar gray_max);
	static void preset0(Camera* camera);
};