#pragma once

#include <opencv2/opencv.hpp>

using namespace cv;

class ThinningComputer
{
	public:
	    int x_min;
	    int x_max;
	    int y_min;
	    int y_max;

		void thinningIteration(cv::Mat& img, int iter);
		void thinning(cv::Mat& src);
};