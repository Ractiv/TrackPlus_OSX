#pragma once

#include <map>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

class LowPassFilter{
	public:
		static map<string, double> value_map;

		static void compute(double& value, const double alpha, const string name);
		static void compute(int& value, const double alpha, const string name);
		static void compute(uchar& value, const double alpha, const string name);
		static void compute(Point& value, const double alpha, const string name);
};