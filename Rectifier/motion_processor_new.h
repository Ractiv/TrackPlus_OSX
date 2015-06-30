#pragma once

#include "globals.h"
#include "blob_detector_new.h"
#include "histogram_builder.h"

class MotionProcessorNew
{
public:
	const int reflection_y = HEIGHT_SMALL * 3 / 4;
	
	int active_frame;
	int two_hands_count_total;
	int current_frame;
	int x_middle;
	int two_hands_count;
	int blob_size_threshold;

	int non_reflection_y_max;
	int non_reflection_y_max_old0;
	int non_reflection_y_max_old1;
	int non_reflection_y_max_final;

	uchar gray_threshold;
	uchar gray_threshold_max;
	uchar gray_threshold_old;
	uchar gray_threshold_diff_max;
	uchar diff_threshold;
	uchar diff_threshold_max;

	double exposure;

	bool disable_one_hand_reconstruction;
	bool disable_count_hands;
	bool one_hand;
	bool two_hands;
	bool moving;
	bool started;
	bool image_background_dynamic_set;
	bool construct_static_background_image;
	bool foreground_acquired;

	Mat image_background_dynamic;
	Mat image_background_static;
	Mat image_preprocessed_old;

	BlobDetectorNew blob_detector_image_subtraction;
	BlobDetectorNew blob_detector_image_segmentation;
	BlobDetectorNew blob_detector_image_histogram_vertical;
	BlobDetectorNew blob_detector_image_histogram_horizontal;
	BlobDetectorNew blob_detector_image_holes;

	HistogramBuilder histogram_builder;

	void init();
	void set_active_frame(const int active_frame_in);
	void compute(Mat& image_in, Mat& image_preprocessed_in, const string name, const bool visualize);
};