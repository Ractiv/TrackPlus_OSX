#pragma once

#include <opencv2/opencv.hpp>
#include <math.h>

using namespace cv;
using namespace std;

double get_distance(double& x0, double& y0, double& x1, double& y1);
double get_distance(const int x0, const int y0, const int x1, const int y1);
double get_distance(Point& pt0, Point& pt1);
double map_val(const double value, const double left_min, const double left_max, const double right_min, const double right_max);
double get_angle(Point& pt0, Point& pt1, Point& pt2);
double get_angle(float& x0, float& y0, float& x1, float& y1);
Point get_intersection_at_y(Point& pt0, Point& pt1, const int i);
double get_mean(vector<uchar>& value_vec);