#include "preprocessor.h"

void Preprocessor::compute_channel_diff_image(Mat&  image_in, Mat& image_out)
{
	const int image_width_const = image_in.cols;
	const int image_height_const = image_in.rows;

	image_out = Mat(image_height_const, image_width_const, CV_8UC1);

	for (int i = 0; i < image_width_const; ++i)
		for (int j = 0; j < image_height_const; ++j)
		{
			int diff0 = image_in.ptr<uchar>(j, i)[0] - image_in.ptr<uchar>(j, i)[1];
			int diff1 = image_in.ptr<uchar>(j, i)[2] - image_in.ptr<uchar>(j, i)[1];

			if (diff0 < 0)
				diff0 = 0;
			if (diff1 < 0)
				diff1 = 0;

			image_out.ptr<uchar>(j, i)[0] = min(diff0, diff1);
		}

	rectangle(image_out, Rect(0, 0, image_in.cols, image_in.rows), Scalar(0), 3);
}

void Preprocessor::compute_max_image(Mat& image_in, Mat&  image_out)
{
	const int image_width_const = image_in.cols;
	const int image_height_const = image_in.rows;

	for (int i = 0; i < image_width_const; ++i)
		for (int j = 0; j < image_height_const; ++j)
			image_out.ptr<uchar>(j, i)[0] = max(image_in.ptr<uchar>(j, i)[0],
										    max(image_in.ptr<uchar>(j, i)[1], image_in.ptr<uchar>(j, i)[2]));
}