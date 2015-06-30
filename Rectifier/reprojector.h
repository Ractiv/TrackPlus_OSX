#pragma once

//#include <direct.h>
// #include <atlstr.h>
// #include <ShlObj.h>
//#include <windows.h>
#include <iostream>
#include <fstream>
#include "Vector3.h"
#include <string.h>
#include <opencv2/opencv.hpp>
#include "globals.h"
#include "math_plus.h"
#include "curve_fitting.h"
#include "stereo_computer.h"
#include "ipc.h"
#include "processes.h"
#include "filesystem.h"

using namespace std;
using namespace cv;

class Reprojector
{
public:
	double a_out;
	double b_out;
	double c_out;
	double d_out;

	Point** rect_mat0 = NULL;
	Point** rect_mat1 = NULL;

	void load(IPC& ipc);
	void proceed();
	double compute_depth(const int disparity_in);
	Point compute_plane_size(double depth);
	Point3d reprojectTo3D(double pt0_x, double pt0_y, double pt1_x, double pt1_y);
	Mat remap(Mat* const image_in, const uchar side, const bool interpolate);
	Mat compute_gray_image(Mat* const image);

	void compute_interaction_plane(Mat* const image0, 			    Mat* const image1,
								   Mat* const image_chasis_mask0,   Mat* const image_chasis_mask1,
								   const int non_reflection_y_max0, const int non_reflection_y_max1);

private:
	struct compare_point_x
	{
		inline bool operator() (const Point pt0_in, const Point pt1_in)
		{
			return pt0_in.x < pt1_in.x;
		}
	};
};