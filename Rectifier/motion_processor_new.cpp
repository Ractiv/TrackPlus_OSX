#include "motion_processor_new.h"

void MotionProcessorNew::init()
{
	set_active_frame(0);

	current_frame = 0;
	x_middle = WIDTH_SMALL / 2;
	two_hands_count = 0;
	blob_size_threshold = 300;

	non_reflection_y_max = 0;
	non_reflection_y_max_old0 = 0;
	non_reflection_y_max_old1 = 0;
	non_reflection_y_max_final = 0;

	gray_threshold = 0;
	gray_threshold_max = 0;
	gray_threshold_old = 0;
	gray_threshold_diff_max = 0;
	diff_threshold = 0;
	diff_threshold_max = 0;

	exposure = 0;

	disable_one_hand_reconstruction = false;
	disable_count_hands = false;
	one_hand = false;
	two_hands = false;
	started = false;
	image_background_dynamic_set = false;
	construct_static_background_image = false;
	foreground_acquired = false;

	image_background_dynamic = Mat(HEIGHT_SMALL, WIDTH_SMALL, CV_8UC3, Scalar(255));
	image_background_static = Mat(HEIGHT_SMALL, WIDTH_SMALL, CV_8UC1, Scalar(255));
	image_preprocessed_old = Mat(HEIGHT_SMALL, WIDTH_SMALL, CV_8UC1, Scalar(255));

	blob_detector_image_subtraction = BlobDetectorNew();
	blob_detector_image_segmentation = BlobDetectorNew();
	blob_detector_image_histogram_vertical = BlobDetectorNew();
	blob_detector_image_histogram_horizontal = BlobDetectorNew();
	blob_detector_image_holes = BlobDetectorNew();

	histogram_builder = HistogramBuilder();
}

void MotionProcessorNew::set_active_frame(const int active_frame_in)
{
	active_frame = active_frame_in;

	if (active_frame_in != 0)
	{
		two_hands_count_total = 20 / active_frame;
		if (two_hands_count_total < 4)
			two_hands_count_total = 4;
	}
	else
		two_hands_count_total = 4;
}

void MotionProcessorNew::compute(Mat& image_in, Mat& image_preprocessed_in, const string name, const bool visualize)
{
	if (current_frame != active_frame)
	{
		++current_frame;
		return;
	}
	current_frame = 0;

	if (image_background_dynamic_set)
	{
		Mat image_subtraction = Mat(HEIGHT_SMALL, WIDTH_SMALL, CV_8UC1);

		for (int i = 0; i < WIDTH_SMALL; ++i)
			for (int j = 0; j < HEIGHT_SMALL; ++j)
			{
				const int diff0 = abs(image_in.ptr<uchar>(j, i)[0] - image_background_dynamic.ptr<uchar>(j, i)[0]);
				const int diff1 = abs(image_in.ptr<uchar>(j, i)[1] - image_background_dynamic.ptr<uchar>(j, i)[1]);
				const int diff2 = abs(image_in.ptr<uchar>(j, i)[2] - image_background_dynamic.ptr<uchar>(j, i)[2]);

				image_subtraction.ptr<uchar>(j, i)[0] = max(max(diff0, diff1), diff2);
			}
		normalize(image_subtraction, image_subtraction, 0, 254, NORM_MINMAX);
		threshold(image_subtraction, image_subtraction, 50, 254, THRESH_BINARY);

		blob_detector_image_subtraction.compute(image_subtraction, 254);

		int white_count = 0;

		for (int i = 0; i < WIDTH_SMALL; ++i)
			for (int j = 0; j < HEIGHT_SMALL; ++j)
				if (image_subtraction.ptr<uchar>(j, i)[0] == 254)
					++white_count;

		if (blob_detector_image_subtraction.blobs.size() < 100 && white_count < 3000)
		{
			Mat image_histogram_vertical;
			histogram_builder.compute_vertical(image_subtraction, image_histogram_vertical, 19);
			blob_detector_image_histogram_vertical.compute(image_histogram_vertical, 254);

			non_reflection_y_max = blob_detector_image_histogram_vertical.blob_max_size->y_max;

			Mat image_histogram_horizontal;
			histogram_builder.compute_horizontal(image_subtraction, image_histogram_horizontal, 49);
			blob_detector_image_histogram_horizontal.compute(image_histogram_horizontal, 254);

			if (!disable_count_hands)
			{
				if (blob_detector_image_histogram_horizontal.blobs.size() == 2)
				{
					if (non_reflection_y_max > reflection_y)
						non_reflection_y_max = non_reflection_y_max_old0;
					else
						non_reflection_y_max_old0 = non_reflection_y_max;

					non_reflection_y_max_old1 = non_reflection_y_max;

					one_hand = false;

					if (two_hands_count >= two_hands_count_total)
					{
						two_hands = true;
						started = true;
					}
					else
						++two_hands_count;
				}
				else
				{
					if (non_reflection_y_max < non_reflection_y_max_old1)
						non_reflection_y_max = non_reflection_y_max_old1;

					else if (non_reflection_y_max > reflection_y)
						non_reflection_y_max = non_reflection_y_max_old0;
					else
						non_reflection_y_max_old0 = non_reflection_y_max;

					one_hand = true;
					two_hands = false;
					two_hands_count = 0;
				}
			}
			else
			{
				one_hand = false;
				two_hands = false;
				two_hands_count = 0;
			}

			if (non_reflection_y_max > non_reflection_y_max_final)
				non_reflection_y_max_final = non_reflection_y_max;
			else
				non_reflection_y_max = non_reflection_y_max_final;

			//find the middle point between two hands

			if (two_hands)
				x_middle = (blob_detector_image_histogram_horizontal.blobs[0].x_max +
						    blob_detector_image_histogram_horizontal.blobs[1].x_min) / 2;

			//find hand shape candidates by matching segmentation image with subtraction image

			if (two_hands)
			{
				const int y_max = non_reflection_y_max / 2;

				int gray_total = 0;
				int gray_count = 0;

				for (BlobNew& blob : blob_detector_image_subtraction.blobs)
					for (Point& pt : blob.data)
						if (pt.y > y_max)
						{
							const uchar gray = max(image_preprocessed_in.ptr<uchar>(pt.y, pt.x)[0],
								                   image_preprocessed_old.ptr<uchar>(pt.y, pt.x)[0]);

							gray_total += gray;
							++gray_count;
						}
				if (gray_count > 0)
				{
					int gray_mean = gray_total / gray_count;
					gray_threshold = gray_mean * 0.8;

					if (gray_threshold > gray_threshold_max)
						gray_threshold_max = gray_threshold;

					if (gray_threshold < gray_threshold_max / 2)
						gray_threshold = gray_threshold_max / 2;
				}
			}
			else if (disable_count_hands)
				gray_threshold = gray_threshold_max;

			Mat image_segmentation;
			threshold(image_preprocessed_in, image_segmentation, gray_threshold, 254, THRESH_BINARY);

			blob_detector_image_segmentation.compute(image_segmentation, 254);
			int blob_count_total = 0;

			for (BlobNew& blob1 : blob_detector_image_segmentation.blobs)
				if (blob1.y < non_reflection_y_max && blob1.count > 10)
				{
					bool overlap = false;
					for (BlobNew& blob0 : blob_detector_image_subtraction.blobs)
						if (blob0.count > 5)
							if (blob1.compute_overlap(blob0, 0, 0) > 0)
							{
								blob_count_total += blob1.count;
								overlap = true;
								break;
							}
					if (!overlap)
					{
						blob1.fill(image_segmentation, 0);
						blob1.active = false;
					}
				}
				else
				{
					blob1.fill(image_segmentation, 0);
					blob1.active = false;
				}

			int max_diff = 0;

			for (BlobNew& blob : blob_detector_image_segmentation.blobs)
				if (blob.active)
					for (Point& pt : blob.data)
						if (image_background_static.ptr<uchar>(pt.y, pt.x)[0] != 255)
						{
							int diff = abs(image_preprocessed_in.ptr<uchar>(pt.y, pt.x)[0] - 
										   image_background_static.ptr<uchar>(pt.y, pt.x)[0]);

							if (diff > max_diff)
								max_diff = diff;

							image_segmentation.ptr<uchar>(pt.y, pt.x)[0] = diff;
						}
			diff_threshold = (max_diff * 0.2) + (exposure * exposure) * 0.2;

			if (diff_threshold > diff_threshold_max)
				diff_threshold_max = diff_threshold;

			if (diff_threshold < diff_threshold_max / 2)
				diff_threshold = diff_threshold_max / 2;

			if (disable_count_hands)
				diff_threshold = diff_threshold_max;

			threshold(image_segmentation, image_segmentation, diff_threshold, 254, THRESH_BINARY);

			blob_detector_image_segmentation.compute(image_segmentation, 254);

			//construct static background image
			if (started && construct_static_background_image && blob_count_total > blob_size_threshold)
			{
				const uchar gray_threshold_diff = abs(gray_threshold - gray_threshold_old);

				if (gray_threshold_diff - gray_threshold_diff_max > 5)
				{
					set_active_frame(10);
					set_active_frame(10);
				}
				if (gray_threshold_old != 0 && gray_threshold != 0 && gray_threshold_diff > gray_threshold_diff_max)
					gray_threshold_diff_max = gray_threshold_diff;

				bool empty_hand_part_of_static_bg_image = false;

				if (one_hand && !disable_one_hand_reconstruction)
				{
					vector<int> x_vec;

					const int j_max = HEIGHT_SMALL * 0.25;
					for (int i = 0; i < WIDTH_SMALL; ++i)
						for (int j = 0; j < j_max; ++j)
							if (image_segmentation.ptr<uchar>(j, i)[0] == 254)
								x_vec.push_back(i);

					if (x_vec.size() > 0)
					{
						sort(x_vec.begin(), x_vec.end());
						const int x_median = x_vec[x_vec.size() / 2];

						int x_min = 9999;
						int x_max = 0;
						for (BlobNew& blob : blob_detector_image_segmentation.blobs)
						{
							if (!blob.active)
								continue;

							if (blob.x_min < x_min)
								x_min = blob.x_min;

							if (blob.x_max > x_max)
								x_max = blob.x_max;
						}
						const int width = x_max - x_min;

						if (width < WIDTH_SMALL / 2)
						{
							int x_begin;
							int x_end;
							if (x_median < x_middle)
							{
								x_begin = 0;
								x_end = x_median;
							}
							else
							{
								x_begin = x_median;
								x_end = WIDTH_SMALL;
							}
							const int i_max = x_end;

							Mat image_segmentation_dilated;
							dilate(image_segmentation, image_segmentation_dilated, Mat(), Point(-1, -1), 5);
							empty_hand_part_of_static_bg_image = true;

							uchar val_foreground = NULL;
							uchar* ptr_background = NULL;

							for (int i = x_begin; i < i_max; ++i)
								for (int j = 0; j < HEIGHT_SMALL; ++j)
									if (image_segmentation_dilated.ptr<uchar>(j, i)[0] == 0)
									{
										ptr_background = &(image_background_static.ptr<uchar>(j, i)[0]);
										val_foreground = image_preprocessed_in.ptr<uchar>(j, i)[0];

										if (*ptr_background == 255)
											*ptr_background = val_foreground;
										else
											*ptr_background = *ptr_background + (val_foreground - *ptr_background) / 1;
									}
						}
					}
				}
				else if (two_hands)
				{
					int left_count = 1;
					int right_count = 1;

					for (BlobNew& blob : blob_detector_image_segmentation.blobs)
						if (blob.x < x_middle)
							left_count += blob.count;
						else
							right_count += blob.count;

					if (right_count > blob_size_threshold && left_count > blob_size_threshold)
					{
						Mat image_segmentation_dilated;
						dilate(image_segmentation, image_segmentation_dilated, Mat(), Point(-1, -1), 5);
						empty_hand_part_of_static_bg_image = true;

						uchar val_foreground = NULL;
						uchar* ptr_background = NULL;

						for (int i = 0; i < WIDTH_SMALL; ++i)
							for (int j = 0; j < HEIGHT_SMALL; ++j)
								if (image_segmentation_dilated.ptr<uchar>(j, i)[0] == 0)
								{
									ptr_background = &(image_background_static.ptr<uchar>(j, i)[0]);
									val_foreground = image_preprocessed_in.ptr<uchar>(j, i)[0];

									if (*ptr_background == 255)
										*ptr_background = val_foreground;
									else
										*ptr_background = *ptr_background + (val_foreground - *ptr_background) / 1;
								}
					}
				}
				if (empty_hand_part_of_static_bg_image && false)
				{
					Mat image_segmentation_removal;
					GaussianBlur(image_segmentation, image_segmentation_removal, Size(29, 29), 0, 0);

					for (int i = 0; i < WIDTH_SMALL; ++i)
						for (int j = HEIGHT_SMALL - 1; j >= 0; --j)
							if (image_segmentation_removal.ptr<uchar>(j, i)[0] > 200)
								image_background_static.ptr<uchar>(j, i)[0] = 255;
				}
				if (!foreground_acquired)
				{
					Mat image_holes = Mat::zeros(HEIGHT_SMALL, WIDTH_SMALL, CV_8UC1);
					for (int i = 0; i < WIDTH_SMALL; ++i)
						for (int j = 0; j < HEIGHT_SMALL; ++j)
							if (image_background_static.ptr<uchar>(j, i)[0] == 255)
								image_holes.ptr<uchar>(j, i)[0] = 254;

					blob_detector_image_holes.compute(image_holes, 254);
					if (blob_detector_image_holes.blobs.size() == 2 && active_frame == 10)
					{
						foreground_acquired = true;
						cout << "foreground acquired" << endl;
					}
				}
			}
			if (visualize)
			{
				line(image_segmentation, Point(x_middle, 0), Point(x_middle, HEIGHT_SMALL), Scalar(127), 1);
				line(image_segmentation, Point(0, non_reflection_y_max), Point(160, non_reflection_y_max), Scalar(127), 1);
				imshow("image_segmentation" + name, image_segmentation);
				imshow("image_subtraction" + name, image_subtraction);
				imshow("image_background_static" + name, image_background_static);
			}
		}
	}
	gray_threshold_old = gray_threshold;
	image_preprocessed_old = image_preprocessed_in;
	image_background_dynamic = image_in;
	image_background_dynamic_set = true;
}