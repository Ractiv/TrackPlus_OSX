#include "blob_detector_new.h"

void BlobDetectorNew::compute(Mat& image_in, const uchar gray_in)
{
	image_atlas = Mat::zeros(image_in.rows, image_in.cols, CV_16UC1);
	Mat image_clone = image_in.clone();

	blobs.clear();
	BlobNew blob_temp;
	blob_max_size = &blob_temp;

	const int j_max = image_in.rows - 2;
	const int i_max = image_in.cols - 2;
	const int y_max = image_in.rows - 1;
	const int x_max = image_in.cols - 1;

	for (int j = 1; j < j_max; ++j)
	{
		for (int i = 1; i < i_max; ++i)
		{
			uchar* pix_ptr = &image_clone.ptr<uchar>(j, i)[0];

			if (*pix_ptr != gray_in)
				continue;

			blobs.push_back(BlobNew(image_atlas, blobs.size() + 1));
			BlobNew* blob = &(blobs[blobs.size() - 1]);
			blob->add(i, j);
			*pix_ptr = 255;

			for (int k = 0; k < blob->count; ++k)
			{
				const int pt_x = blob->data[k].x;
				const int pt_y = blob->data[k].y;

				if (pt_x <= 0 || pt_x >= x_max || pt_y <= 0 || pt_y >= y_max)
					continue;

				const int pt_x0 = pt_x - 1;
				const int pt_y0 = pt_y - 1;
				const int pt_x1 = pt_x + 1;
				const int pt_y1 = pt_y + 1;

				pix_ptr = &image_clone.ptr<uchar>(pt_y, pt_x0)[0];
				if (*pix_ptr == gray_in)
				{
					blob->add(pt_x0, pt_y);
					*pix_ptr = 255;
				}
				pix_ptr = &image_clone.ptr<uchar>(pt_y, pt_x1)[0];
				if (*pix_ptr == gray_in)
				{
					blob->add(pt_x1, pt_y);
					*pix_ptr = 255;
				}
				pix_ptr = &image_clone.ptr<uchar>(pt_y0, pt_x)[0];
				if (*pix_ptr == gray_in)
				{
					blob->add(pt_x, pt_y0);
					*pix_ptr = 255;
				}
				pix_ptr = &image_clone.ptr<uchar>(pt_y1, pt_x)[0];
				if (*pix_ptr == gray_in)
				{
					blob->add(pt_x, pt_y1);
					*pix_ptr = 255;
				}

				pix_ptr = &image_clone.ptr<uchar>(pt_y0, pt_x0)[0];
				if (*pix_ptr == gray_in)
				{
					blob->add(pt_x0, pt_y0);
					*pix_ptr = 255;
				}
				pix_ptr = &image_clone.ptr<uchar>(pt_y1, pt_x1)[0];
				if (*pix_ptr == gray_in)
				{
					blob->add(pt_x1, pt_y1);
					*pix_ptr = 255;
				}
				pix_ptr = &image_clone.ptr<uchar>(pt_y0, pt_x1)[0];
				if (*pix_ptr == gray_in)
				{
					blob->add(pt_x1, pt_y0);
					*pix_ptr = 255;
				}
				pix_ptr = &image_clone.ptr<uchar>(pt_y1, pt_x0)[0];
				if (*pix_ptr == gray_in)
				{
					blob->add(pt_x0, pt_y1);
					*pix_ptr = 255;
				}
			}
			blob->compute();
			if (blob->count > blob_max_size->count)
				blob_max_size = blob;
		}
	}
}

void BlobDetectorNew::compute_location(Mat& image_in, const uchar gray_in, const int i, const int j)
{
	image_atlas = Mat::zeros(image_in.rows, image_in.cols, CV_16UC1);
	Mat image_clone = image_in.clone();

	blobs.clear();
	BlobNew blob_temp;
	blob_max_size = &blob_temp;

	const int j_max = image_in.rows - 2;
	const int i_max = image_in.cols - 2;
	const int y_max = image_in.rows - 1;
	const int x_max = image_in.cols - 1;

	uchar* pix_ptr = &image_clone.ptr<uchar>(j, i)[0];

	if (*pix_ptr != gray_in)
		return;

	blobs.push_back(BlobNew(image_atlas, blobs.size() + 1));
	BlobNew* blob = &(blobs[blobs.size() - 1]);
	blob->add(i, j);
	*pix_ptr = 255;

	for (int k = 0; k < blob->count; ++k)
	{
		const int pt_x = blob->data[k].x;
		const int pt_y = blob->data[k].y;

		if (pt_x <= 0 || pt_x >= x_max || pt_y <= 0 || pt_y >= y_max)
			continue;

		const int pt_x0 = pt_x - 1;
		const int pt_y0 = pt_y - 1;
		const int pt_x1 = pt_x + 1;
		const int pt_y1 = pt_y + 1;

		pix_ptr = &image_clone.ptr<uchar>(pt_y, pt_x0)[0];
		if (*pix_ptr == gray_in)
		{
			blob->add(pt_x0, pt_y);
			*pix_ptr = 255;
		}
		pix_ptr = &image_clone.ptr<uchar>(pt_y, pt_x1)[0];
		if (*pix_ptr == gray_in)
		{
			blob->add(pt_x1, pt_y);
			*pix_ptr = 255;
		}
		pix_ptr = &image_clone.ptr<uchar>(pt_y0, pt_x)[0];
		if (*pix_ptr == gray_in)
		{
			blob->add(pt_x, pt_y0);
			*pix_ptr = 255;
		}
		pix_ptr = &image_clone.ptr<uchar>(pt_y1, pt_x)[0];
		if (*pix_ptr == gray_in)
		{
			blob->add(pt_x, pt_y1);
			*pix_ptr = 255;
		}

		pix_ptr = &image_clone.ptr<uchar>(pt_y0, pt_x0)[0];
		if (*pix_ptr == gray_in)
		{
			blob->add(pt_x0, pt_y0);
			*pix_ptr = 255;
		}
		pix_ptr = &image_clone.ptr<uchar>(pt_y1, pt_x1)[0];
		if (*pix_ptr == gray_in)
		{
			blob->add(pt_x1, pt_y1);
			*pix_ptr = 255;
		}
		pix_ptr = &image_clone.ptr<uchar>(pt_y0, pt_x1)[0];
		if (*pix_ptr == gray_in)
		{
			blob->add(pt_x1, pt_y0);
			*pix_ptr = 255;
		}
		pix_ptr = &image_clone.ptr<uchar>(pt_y1, pt_x0)[0];
		if (*pix_ptr == gray_in)
		{
			blob->add(pt_x0, pt_y1);
			*pix_ptr = 255;
		}
	}
	blob->compute();
	if (blob->count > blob_max_size->count)
		blob_max_size = blob;
}

void BlobDetectorNew::sort_blobs_by_count()
{
	sort(blobs.begin(), blobs.end(), compare_blob_count());
}

void BlobDetectorNew::sort_blobs_by_distance(const int x_in, const int y_in)
{
	sort(blobs.begin(), blobs.end(), compare_blob_distance(x_in, y_in));
}