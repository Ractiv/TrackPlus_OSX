#include "low_pass_filter.h"

map<string, double> LowPassFilter::value_map = map<string, double>();

void LowPassFilter::compute(double& value, const double alpha, const string name)
{
	const double value_old = value_map[name];
	const double value_new = value;
	double result = value_old + ((value_new - value_old) * alpha);

	if (result == value_new / 2)
		result = value_new;
	
	value_map[name] = result;
	value = result;
}

void LowPassFilter::compute(int& value, const double alpha, const string name)
{
	const double value_old = value_map[name];
	const double value_new = value;
	double result = value_old + ((value_new - value_old) * alpha);

	if (result == value_new / 2)
		result = value_new;

	value_map[name] = result;
	value = result;
}

void LowPassFilter::compute(uchar& value, const double alpha, const string name)
{
	const double value_old = value_map[name];
	const double value_new = value;
	double result = value_old + ((value_new - value_old) * alpha);

	if (result == value_new / 2)
		result = value_new;

	value_map[name] = result;
	value = result;
}

void LowPassFilter::compute(Point& value, const double alpha, const string name)
{
	const string name_x = name + "x";
	const string name_y = name + "y";

	const double value_old_x = value_map[name_x];
	const double value_new_x = value.x;
	const double value_old_y = value_map[name_y];
	const double value_new_y = value.y;

	double result_x = value_old_x + ((value_new_x - value_old_x) * alpha);
	double result_y = value_old_y + ((value_new_y - value_old_y) * alpha);

	value_map[name_x] = result_x;
	value_map[name_y] = result_y;

	value.x = result_x;
	value.y = result_y;
}