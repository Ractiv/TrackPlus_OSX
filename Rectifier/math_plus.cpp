#include "math_plus.h"

double get_distance(double& x0, double& y0, double& x1, double& y1)
{
	return sqrt(pow(x0 - x1, 2) + pow(y0 - y1, 2));
}

double get_distance(const int x0, const int y0, const int x1, const int y1)
{
	return sqrt(pow(x0 - x1, 2) + pow(y0 - y1, 2));
}

double get_distance(Point& pt0, Point& pt1)
{
	return sqrt(pow(pt0.x - pt1.x, 2) + pow(pt0.y - pt1.y, 2));
}

double map_val(const double value, const double left_min, const double left_max, const double right_min, const double right_max)
{
    double left_span = left_max - left_min;
    double right_span = right_max - right_min;
    double valueScaled = (value - left_min) / (left_span);
    return right_min + valueScaled * right_span;
}

double get_angle(Point& p1, Point& p2, Point& p3)
{
	double p12 = get_distance(p1, p2);
	double p13 = get_distance(p1, p3);
	double p23 = get_distance(p2, p3);
	return acos((pow(p12, 2) + pow(p13, 2) - pow(p23, 2)) / (2 * p12 * p13)) * 180 / CV_PI;
}

double get_angle(float& x0, float& y0, float& x1, float& y1)
{
	return atan2(y1 - y0, x1 - x0) * 180.0 / CV_PI;
}

Point get_intersection_at_y(Point& pt0, Point& pt1, const int y)
{
	Point pt_y_min;
	Point pt_y_max;

	if (pt0.y < pt1.y)
	{
		pt_y_min = pt0;
		pt_y_max = pt1;
	}
	else
	{
		pt_y_min = pt1;
		pt_y_max = pt0;
	}
	double y_diff = pt_y_max.y - pt_y_min.y;
	double x_diff = pt_y_max.x - pt_y_min.x;
	double y_diff_from_y = pt_y_min.y - y;
	double scale = y_diff_from_y / y_diff;
	double x_diff_scaled = x_diff * scale;
	int x = pt_y_min.x - x_diff_scaled;

	return Point(x, y);
}

double get_mean(vector<uchar>& value_vec)
{
	double result = 0;
	for (uchar& val : value_vec)
		result += val;

	result /= value_vec.size();
	return result;
}