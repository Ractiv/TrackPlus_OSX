#include "stereo_computer.h"

using namespace std;

vector<Match*> StereoComputer::getMatchingPoints(const Mat img_1, const Mat img_2, int drawSIFTPoints, int drawResult)
{
	SurfFeatureDetector detector(1500);

	vector<KeyPoint> keypoints_1;
	vector<KeyPoint> keypoints_2;

	detector.detect(img_1, keypoints_1);
	detector.detect(img_2, keypoints_2);

	if (drawSIFTPoints)
	{
		Mat img_keypoints_1; Mat img_keypoints_2;
		drawKeypoints(img_1, keypoints_1, img_keypoints_1, Scalar::all(-1), DrawMatchesFlags::DEFAULT);
		drawKeypoints(img_2, keypoints_2, img_keypoints_2, Scalar::all(-1), DrawMatchesFlags::DEFAULT);
		imshow("Keypoints 1", img_keypoints_1);
		moveWindow("Keypoints 1", 0, 500);
		imshow("Keypoints 2", img_keypoints_2);
		moveWindow("Keypoints 2", 660, 500);
	}

	SiftDescriptorExtractor extractor;
	Mat descriptors_1, descriptors_2;

	extractor.compute(img_1, keypoints_1, descriptors_1);
	extractor.compute(img_2, keypoints_2, descriptors_2);

	FlannBasedMatcher matcher;
	vector<DMatch> matches;
	matcher.match(descriptors_1, descriptors_2, matches);

	double max_dist = 0;
	double min_dist = 100;

	for (int i = 0; i < descriptors_1.rows; i++)
	{
		double dist = matches[i].distance;
		if (dist < min_dist) min_dist = dist;
		if (dist > max_dist) max_dist = dist;
	}
	vector<DMatch> good_matches;

	for (int i = 0; i < descriptors_1.rows; i++)
		// if (matches[i].distance <= max(2 * min_dist, 0.02))
			good_matches.push_back(matches[i]);

	if (drawResult)
	{
		Mat img_matches;
		drawMatches(img_1, keypoints_1, img_2, keypoints_2,
					good_matches, img_matches, Scalar::all(-1), Scalar::all(-1),
					vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);

		//-- Show detected matches
		imshow("Good Matches", img_matches);
		moveWindow("Good Matches", 0, 500);
	}
	vector<Match*> matchesToReturn;
	for (int i = 0; i < (int)good_matches.size(); i++)
	{
		Match * temp = new Match();
		temp->img1_pt = Point(keypoints_1[good_matches[i].queryIdx].pt.x, keypoints_1[good_matches[i].queryIdx].pt.y);
		temp->img2_pt = Point(keypoints_2[good_matches[i].trainIdx].pt.x, keypoints_2[good_matches[i].trainIdx].pt.y);
		matchesToReturn.push_back(temp);
	}
	return matchesToReturn;
}