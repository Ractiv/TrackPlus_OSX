#pragma once

#include "blob_new.h"
#include "globals.h"
#include "math_plus.h"

class BlobDetectorNew
{
public:
	vector<BlobNew> blobs;
	BlobNew* blob_max_size = NULL;

	Mat image_atlas;

	void compute(Mat& image_in, const uchar gray_in);
	void compute_location(Mat& image_in, const uchar gray_in, const int i, const int j);
	void sort_blobs_by_count();
	void sort_blobs_by_distance(const int x_in, const int y_in);

private:
	struct compare_blob_count
	{
		bool operator() (const BlobNew& blob0, const BlobNew& blob1)
		{
			return (blob0.count > blob1.count);
		}
	};

	struct compare_blob_distance
	{
		int x;
		int y;

		compare_blob_distance(const int x_in, const int y_in)
		{
			x = x_in;
			y = y_in;
		}

		bool operator() (const BlobNew& blob0, const BlobNew& blob1)
		{
			return get_distance(blob0.x, blob0.y, x, y) < get_distance(blob1.x, blob1.y, x, y);
		}
	};
};