#include "foreground_extractor_new.h"

void ForegroundExtractorNew::init()
{
	blob_detector = BlobDetectorNew();
	image_foreground = Mat();
}

void ForegroundExtractorNew::compute(Mat& image_in, MotionProcessorNew& motion_processor, const string name, const bool visualize)
{
	if (!motion_processor.started)
		return;

	image_foreground = Mat::zeros(HEIGHT_SMALL, WIDTH_SMALL, CV_8UC1);
	int diff_max = 0;

	for (int i = 0; i < WIDTH_SMALL; ++i)
		for (int j = 0; j < HEIGHT_SMALL; ++j)
		{
			const uchar gray_current = image_in.ptr<uchar>(j, i)[0];

			if (gray_current > motion_processor.gray_threshold)
			{
				if (motion_processor.image_background_static.ptr<uchar>(j, i)[0] == 255)
					image_foreground.ptr<uchar>(j, i)[0] = 254;

				else
				{
					int diff = abs(gray_current - motion_processor.image_background_static.ptr<uchar>(j, i)[0]);
					if (diff > diff_max)
						diff_max = diff;

					image_foreground.ptr<uchar>(j, i)[0] = diff;
				}
			}
		}
	threshold(image_foreground, image_foreground, motion_processor.diff_threshold, 254, THRESH_BINARY);

	GaussianBlur(image_foreground, image_foreground, Size(1, 5), 0, 0);
	threshold(image_foreground, image_foreground, 100, 254, THRESH_BINARY);

	blob_detector.compute(image_foreground, 254);

	for (BlobNew& blob : blob_detector.blobs)
		if (blob.y > motion_processor.non_reflection_y_max)
		{
			blob.fill(image_foreground, 0);
			blob.active = false;
		}
		else if (blob.count < 5)
		{
			blob.fill(image_foreground, 0);
			blob.active = false;
		}

	bool hit = true;

	while (hit)
	{
		hit = false;
		for (int i = 0; i < WIDTH_SMALL; ++i)
			if (image_foreground.ptr<uchar>(motion_processor.non_reflection_y_max, i)[0] == 254)
				if (motion_processor.non_reflection_y_max < HEIGHT_SMALL - 1)
				{
					++(motion_processor.non_reflection_y_max);
					hit = true;
					break;
				}
				else
					break;
	}
	if (motion_processor.one_hand == false)
	{
		if (motion_processor.non_reflection_y_max > motion_processor.reflection_y)
			motion_processor.non_reflection_y_max = motion_processor.non_reflection_y_max_old0;
		else
			motion_processor.non_reflection_y_max_old0 = motion_processor.non_reflection_y_max;

		motion_processor.non_reflection_y_max_old1 = motion_processor.non_reflection_y_max;
	}
	else
	{
		if (motion_processor.non_reflection_y_max < motion_processor.non_reflection_y_max_old1)
			motion_processor.non_reflection_y_max = motion_processor.non_reflection_y_max_old1;

		else if (motion_processor.non_reflection_y_max > motion_processor.reflection_y)
			motion_processor.non_reflection_y_max = motion_processor.non_reflection_y_max_old0;
		else
			motion_processor.non_reflection_y_max_old0 = motion_processor.non_reflection_y_max;
	}

	if (visualize && background_set)
	{
		Mat image_structure = Mat(HEIGHT_SMALL, WIDTH_SMALL, CV_8UC1);

		for (int i = 0; i < WIDTH_SMALL; ++i)
			for (int j = 0; j < HEIGHT_SMALL; ++j)
				image_structure.ptr<uchar>(j, i)[0] = ((int)image_in.ptr<uchar>(j, i)[0] - image_background.ptr<uchar>(j, i)[0]) + 127;

		equalizeHist(image_structure, image_structure);
		dilate(image_foreground, image_foreground, Mat(), Point(-1, -1), 2);

		for (int i = 0; i < WIDTH_SMALL; ++i)
			for (int j = 0; j < HEIGHT_SMALL; ++j)
				if (image_foreground.ptr<uchar>(j, i)[0] == 0)
					image_structure.ptr<uchar>(j, i)[0] = 0;

		imshow("image_structure" + name, image_structure);
	}
	image_background = image_in;
	background_set = true;
}