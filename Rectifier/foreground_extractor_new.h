#pragma once

#include "motion_processor_new.h"

class ForegroundExtractorNew
{
public:
	BlobDetectorNew blob_detector;

	Mat image_foreground;
	Mat image_background;

	bool background_set = false;

	void init();
	void compute(Mat& image_in, MotionProcessorNew& motion_processor, const string name, const bool visualize);
};