#pragma once

#include <opencv2/opencv.hpp>
#include "globals.h"

using namespace std;
using namespace cv;

class BlobNew
{
public:
	Mat image_atlas;

	ushort atlas_id;

	vector<Point> data;

	int x_min = 9999;
	int x_max = 0;
	int y_min = 9999;
	int y_max = 0;
	int width;
	int height;
	int area;
	int count = 0;
	int x;
	int y;

	bool active = true;

	//items required by mono_processor
	Point pt_intersection0;
	Point pt_intersection1;
	Point pt_intersection2;
	Point pt_intersection3;

	int width_proper;
	int x0;
	int y0;

	double dist;

	BlobNew* matching_blob = NULL;
	//

	BlobNew();
	BlobNew(Mat& image_atlas_in, const ushort atlas_id_in);

	void add(const int i, const int j);
	void compute();
	int compute_overlap(BlobNew& blob_in, const int x_diff_in, const int y_diff_in);
	void fill(Mat& image_in, const uchar gray_in);
};