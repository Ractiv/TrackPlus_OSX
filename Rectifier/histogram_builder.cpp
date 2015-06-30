#include "histogram_builder.h"

void HistogramBuilder::compute_vertical(Mat& image_in, Mat& image_out, const int gaussian_val)
{
	const int height_small = image_in.rows;
	const int width_small = image_in.cols;

	image_out = Mat::zeros(height_small, width_small, CV_8UC1);

	for (int j = 0; j < height_small; ++j)
	{
		int count = 0;
		for (int i = 0; i < width_small; ++i)
			if (image_in.ptr<uchar>(j, i)[0] == 254)
				++count;

		if (count > 0)
			line(image_out, Point(0, j), Point(count, j), Scalar(254), 1);
	}
	GaussianBlur(image_out, image_out, Size(gaussian_val, gaussian_val), 0, 0);
	threshold(image_out, image_out, 150, 254, THRESH_BINARY);
}

void HistogramBuilder::compute_horizontal(Mat& image_in, Mat& image_out, const int gaussian_val)
{
	const int height_small = image_in.rows;
	const int width_small = image_in.cols;
	
	image_out = Mat::zeros(height_small, width_small, CV_8UC1);

	for (int i = 0; i < width_small; ++i)
	{
		int count = 0;
		for (int j = 0; j < height_small; ++j)
			if (image_in.ptr<uchar>(j, i)[0] == 254)
				++count;

		if (count > 0)
			line(image_out, Point(i, 0), Point(i, count), Scalar(254), 1);
	}
	GaussianBlur(image_out, image_out, Size(gaussian_val, gaussian_val), 0, 0);
	threshold(image_out, image_out, 150, 254, THRESH_BINARY);
}