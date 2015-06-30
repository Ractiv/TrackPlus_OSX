#include "blob_new.h"

BlobNew::BlobNew(){}

BlobNew::BlobNew(Mat& image_atlas_in, const ushort atlas_id_in)
{
	image_atlas = image_atlas_in;
	atlas_id = atlas_id_in;
}

void BlobNew::add(const int i_in, const int j_in)
{
	data.push_back(Point(i_in, j_in));

	if (i_in < x_min)
		x_min = i_in;
	if (i_in > x_max)
		x_max = i_in;
	if (j_in < y_min)
		y_min = j_in;
	if (j_in > y_max) 
		y_max = j_in;

	++count;
	image_atlas.ptr<uchar>(j_in, i_in)[0] = atlas_id;
}

void BlobNew::compute()
{
	width = x_max - x_min;
	height = y_max - y_min;
	area = width * height;
	x = (x_max - x_min) / 2 + x_min;
	y = (y_max - y_min) / 2 + y_min;
}

int BlobNew::compute_overlap(BlobNew& blob_in, const int x_diff_in, const int y_diff_in)
{
	int overlap_count = 0;
	for (Point pt : data)
		if (blob_in.image_atlas.ptr<uchar>(pt.y, pt.x)[0] == blob_in.atlas_id)
			++overlap_count;

	return overlap_count;
}

void BlobNew::fill(Mat& image_in, const uchar gray_in)
{
	for (Point pt : data)
		image_in.ptr<uchar>(pt.y, pt.x)[0] = gray_in;
}