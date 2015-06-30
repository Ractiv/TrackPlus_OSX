#pragma once

#include "globals.h"
#include "blob_detector_new.h"
#include "math_plus.h"
#include "filesystem.h"
#include "motion_processor_new.h"
#include "foreground_extractor_new.h"
#include "histogram_builder.h"

class HandSplitterNew
{
public:
	BlobDetectorNew blob_detector;

	HistogramBuilder histogram_builder;

	BlobNew blob_old0;
	BlobNew blob_old1;

	int blob_count_old;
	int count;
	int x_min_on_merge;
	int x_max_on_merge;
	int width_on_merge;
	int x_middle_median;

	Mat image_active_hand;

	vector<BlobNew> blob_vec127;
	vector<BlobNew> blob_vec63;

	vector<int> x_middle_vec;

	void init();
	bool compute(ForegroundExtractorNew& foreground_extractor, MotionProcessorNew& motion_processor);
	void save_image();
};