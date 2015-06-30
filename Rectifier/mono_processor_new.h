#pragma once

#include "blob_detector_new.h"
#include "thinning_computer.h"
#include "low_pass_filter.h"
#include "math_plus.h"

class MonoProcessorNew
{
public:
	BlobDetectorNew blob_detector0;
	BlobDetectorNew blob_detector1;
	BlobDetectorNew blob_detector2;

	ThinningComputer thinning_computer;

	LowPassFilter low_pass_filter;

	void init();
	void compute(Mat& image_in, vector<BlobNew>& blobs_in);
	void bresenham(int x1_in, int y1_in, int const x2_in, int const y2_in, vector<Point>& result_out, const uchar count_in);
	void parse_gesture(Mat& image_in, vector<BlobNew>& fingertip_blobs, const int white_pix_count_total, vector<BlobNew>& skeleton_parts,
					   Mat& image_palm_find_contours);

private:
	struct compare_contour_angle
	{
		Point sort_pivot;

		compare_contour_angle(Point& sort_pivot_in)
		{
			sort_pivot = sort_pivot_in;
		};

		Point compute_median_point(vector<Point>& contour_in)
		{
			vector<int> x_vec;
			vector<int> y_vec;

			for (Point& pt : contour_in)
			{
				x_vec.push_back(pt.x);
				y_vec.push_back(pt.y);
			}
			sort(x_vec.begin(), x_vec.end());
			sort(y_vec.begin(), y_vec.end());

			const int x_median = x_vec[x_vec.size() / 2];
			const int y_median = y_vec[y_vec.size() / 2];
			return Point(x_median, y_median);
		};

		bool operator() (vector<Point>& contour0_in, vector<Point> contour1_in)
		{
			Point pt0 = compute_median_point(contour0_in);
			Point pt1 = compute_median_point(contour1_in);
			double val0 = atan2(pt0.y - sort_pivot.y, pt0.x - sort_pivot.x);
			double val1 = atan2(pt1.y - sort_pivot.y, pt1.x - sort_pivot.x);
			return val0 > val1;
		};
	};

	struct compare_matching_blob_y
	{
		bool operator() (const BlobNew& blob0_in, const BlobNew& blob1_in)
		{
            return blob0_in.matching_blob->y > blob1_in.matching_blob->y;
		};
	}; 
};