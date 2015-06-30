#pragma once

#include "opencv2/opencv.hpp"
#include <opencv2/nonfree/features2d.hpp>
#include <opencv2/nonfree/nonfree.hpp>

using namespace cv;

class Match
{
public:
	Point img1_pt;
	Point img2_pt;
};

class StereoComputer
{
public:
	vector<Match*> getMatchingPoints(const Mat img_1, const Mat img_2, int drawSIFTPoints, int drawResult);
};