#include "hand_splitter_new.h"

void HandSplitterNew::init()
{
	blob_detector = BlobDetectorNew();

	histogram_builder = HistogramBuilder();

	BlobNew blob_old0 = BlobNew();
	BlobNew blob_old1 = BlobNew();

	blob_count_old = 0;
	count = 0;
	x_min_on_merge = 0;
	x_max_on_merge = 0;
	width_on_merge = 0;
	x_middle_median = 0;

	image_active_hand = Mat(HEIGHT_SMALL, WIDTH_SMALL, CV_8UC1);

	blob_vec127 = vector<BlobNew>();
	blob_vec63 = vector<BlobNew>();

	x_middle_vec = vector<int>();
}

bool HandSplitterNew::compute(ForegroundExtractorNew& foreground_extractor, MotionProcessorNew& motion_processor)
{
	Mat image_in = foreground_extractor.image_foreground;

	Mat image_small;
	resize(image_in, image_small, Size(WIDTH_SMALL / 2, HEIGHT_SMALL / 2), 0, 0, INTER_LINEAR);

	Mat image_dilated;
	dilate(image_small, image_dilated, Mat(), Point(-1, -1), 1);

	Mat image_blobs;
	histogram_builder.compute_horizontal(image_dilated, image_blobs, 19);

	blob_detector.compute(image_blobs, 254);
	for (BlobNew& blob : blob_detector.blobs)
	{
		blob.width *= 2;
		blob.height *= 2;
		blob.x *= 2;
		blob.y *= 2;
		blob.x_min *= 2;
		blob.x_max *= 2;
		blob.y_min *= 2;
		blob.y_max *= 2;
	}

	int blob_count_new = blob_detector.blobs.size() > 2 ? 2 : blob_detector.blobs.size();
	bool merge = false;

	if (blob_count_new == 0)
	{
		blob_old0.active = false;
		blob_old1.active = false;
		merge = false;
		count = 0;
	}
	else if (blob_count_new == 2)
	{
		blob_old0 = blob_detector.blobs[0];
		blob_old1 = blob_detector.blobs[1];
		merge = false;
		count = 2;
	}
	else if (blob_count_new == 1 && blob_count_old == 2)
	{	
		if (blob_detector.blobs[0].count - max(blob_old0.count, blob_old1.count) > 200)
		{
			count = 2;
			merge = true;
		}
		else
		{
			count = 1;
			merge = false;
		}
	}
	else if (blob_count_new == 1 && blob_count_old == 0)
		count = 1;

	bool merged = (blob_detector.blobs.size() == 1 && count == 2);

	if (motion_processor.two_hands)
		count = 2;

	blob_count_old = blob_count_new;

	if (count == 1)
	{
		motion_processor.disable_one_hand_reconstruction = false;

		if (!merge)
		{
			int y_max = 0;
			int y_min = 9999;

			for (int i = 0; i < WIDTH_SMALL; ++i)
				for (int j = 0; j < HEIGHT_SMALL; ++j)
					if (image_in.ptr<uchar>(j, i)[0] == 254)
					{
						if (j > y_max)
							y_max = j;
						if (j < y_min)
							y_min = j;
					}
			const int y_cap = (y_max - y_min) * 0.25 + y_min;
			vector<int> x_vec;

			for (int i = 0; i < WIDTH_SMALL; ++i)
				for (int j = y_min; j <= y_cap; ++j)
					if (image_in.ptr<uchar>(j, i)[0] == 254)
						x_vec.push_back(i);

			sort(x_vec.begin(), x_vec.end());
			const int x_pivot = x_vec[x_vec.size() / 2];

			if (x_pivot > x_middle_median)
				motion_processor.x_middle = blob_detector.blobs[0].x_min - (blob_detector.blobs[0].width / 4);
			else
				motion_processor.x_middle = blob_detector.blobs[0].x_max + (blob_detector.blobs[0].width / 4);
		}
	}
	else
		motion_processor.disable_one_hand_reconstruction = true;

	if (count == 0)
		motion_processor.disable_count_hands = true;
	else
		motion_processor.disable_count_hands = false;

	image_active_hand = Mat::zeros(HEIGHT_SMALL, WIDTH_SMALL, CV_8UC1);
	blob_vec127 = vector<BlobNew>();
	blob_vec63 = vector<BlobNew>();

	if (merge)
	{
		//record xmin, xmax, ymax of 2 hands, and shift center line hereafter accordingly
		x_min_on_merge = 9999;
		x_max_on_merge = 0;

		for (BlobNew& blob : blob_detector.blobs)
		{
			if (blob.x_min < x_min_on_merge)
				x_min_on_merge = blob.x_min;
			if (blob.x_max > x_max_on_merge)
				x_max_on_merge = blob.x_max;
		}
		width_on_merge = x_max_on_merge - x_min_on_merge;
	}
	else if (merged)
	{
		int x_min_current = 9999;
		int x_max_current = 0;

		for (BlobNew& blob : blob_detector.blobs)
		{
			if (blob.x_min < x_min_current)
				x_min_current = blob.x_min;
			if (blob.x_max > x_max_current)
				x_max_current = blob.x_max;
		}
		const int width_current = x_max_current - x_min_current;

		if (abs(width_current - width_on_merge) >= 4)
		{
			//stop tracking
		}
	}

	if (count == 2 && !merged)
	{
		BlobNew* blob_left;
		BlobNew* blob_right;

		if (blob_detector.blobs[0].x < blob_detector.blobs[1].x)
		{
			blob_left = &blob_detector.blobs[0];
			blob_right = &blob_detector.blobs[1];
		}
		else
		{
			blob_left = &blob_detector.blobs[1];
			blob_right = &blob_detector.blobs[0];
		}
		motion_processor.x_middle = (blob_right->x_min + blob_left->x_max) / 2;

		if (x_middle_vec.size() == 1000)
			x_middle_vec.erase(x_middle_vec.begin());

		x_middle_vec.push_back(motion_processor.x_middle);

		vector<int> x_middle_vec_sorted = x_middle_vec;
		sort(x_middle_vec_sorted.begin(), x_middle_vec_sorted.end());
		x_middle_median = x_middle_vec_sorted[x_middle_vec_sorted.size() / 2];
	}

	/*for (int i = 0; i < WIDTH_SMALL; ++i)
		for (int j = 0; j < HEIGHT_SMALL; ++j)
			if (image_in.ptr<uchar>(j, i)[0] == 254)
				if (i < motion_processor.x_middle)
					image_active_hand.ptr<uchar>(j, i)[0] = 254;
				else
					image_active_hand.ptr<uchar>(j, i)[0] = 127;*/

	line(image_in, Point(motion_processor.x_middle, 0), Point(motion_processor.x_middle, HEIGHT_SMALL), Scalar(254), 1);
	line(image_in, Point(x_middle_median, 0), Point(x_middle_median, HEIGHT_SMALL), Scalar(127), 1);
	imshow("image_in", image_in);
	imshow("image_blobs", image_blobs);

	return false;
}

void HandSplitterNew::save_image()
{
	if (!directory_exists(pose_database_path))
		create_directory(pose_database_path);

	static bool first_save = true;
	static int file_name_vec_size = 0;

	if (first_save)
	{
		first_save = false;
		vector<string> file_name_vec = list_files_in_directory(pose_database_path);
		file_name_vec_size = file_name_vec.size();
	}
	imwrite(pose_database_path, image_active_hand);
}