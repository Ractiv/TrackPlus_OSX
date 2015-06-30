#pragma once

#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

class Preprocessor
{
public:
	static void compute_channel_diff_image(Mat&  image_in, Mat&  image_out);
	static void compute_max_image(Mat&  image_in, Mat& image_out);
};