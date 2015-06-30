#pragma once

#include <opencv2/opencv.hpp>
#include "globals.h"

using namespace cv;

class HistogramBuilder
{
public:
	void compute_vertical(Mat& image_in, Mat& image_out, const int gaussian_val);
	void compute_horizontal(Mat& image_in, Mat& image_out, const int gaussian_val);
};