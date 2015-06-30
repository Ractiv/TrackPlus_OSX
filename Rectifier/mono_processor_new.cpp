#include "mono_processor_new.h"

void MonoProcessorNew::init()
{
	blob_detector0 = BlobDetectorNew();
	blob_detector1 = BlobDetectorNew();
	blob_detector2 = BlobDetectorNew();

	thinning_computer = ThinningComputer();

	low_pass_filter = LowPassFilter();
}

void MonoProcessorNew::compute(Mat& image_in, vector<BlobNew>& blobs_in)
{
	Point pt_corner = Point(WIDTH_SMALL, 0);
	Point pivot;

	int weight_min = 9999;
	int white_pix_count_total = 0;

	for (BlobNew& blob : blobs_in)
	{
		white_pix_count_total += blob.count;

		for (Point& pt : blob.data)
		{
			int weight = abs(pt.x - pt_corner.x) + abs(pt.y - pt_corner.y) * 2;
			if (weight < weight_min)
			{
				weight_min = weight;
				pivot = pt;
			}
		}
	}
	vector<Vec4i> hiearchy;
	vector<vector<Point>> contours;
	Mat image_find_contours = image_in.clone();
	findContours(image_find_contours, contours, hiearchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE, Point(0, 0));

	if (contours.size() > 1)
	{
		sort(contours.begin(), contours.end(), compare_contour_angle(pivot));
		vector<Point>* contour_old = &contours[0];
		vector<Point>* contour_new;
		
		const int contours_size = contours.size();
		for (int i = 1; i < contours_size; ++i)
		{
			contour_new = &contours[i];

			double dist_min = 9999;
			Point* pt_dist_min0 = NULL;
			Point* pt_dist_min1 = NULL;

			for (Point& pt0 : *contour_new)
				for (Point& pt1 : *contour_old)
				{
					const double dist = get_distance(pt0, pt1);

					if (dist < dist_min)
					{
						dist_min = dist;
						pt_dist_min0 = &pt0;
						pt_dist_min1 = &pt1;
					}
				}
			line(image_in, *pt_dist_min0, *pt_dist_min1, Scalar(254), 1);
			contour_old = contour_new;
		}
		image_find_contours = image_in.clone();
		findContours(image_find_contours, contours, hiearchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE, Point(0, 0));
	}
	else if (contours.size() == 0)
		return;

	Mat image_thinning = image_in.clone();
	thinning_computer.thinning(image_thinning);

	vector<Point>* contour = &contours[0];

	double dist_min_current = 9999;
	int index_dist_min;

	const int contour_size = contour->size();
	for (int i = 0; i < contour_size; ++i)
	{
		const double dist = get_distance((*contour)[i], pivot);
		if (dist < dist_min_current)
		{
			dist_min_current = dist;
			index_dist_min = i;
		}
	}
	vector<Point> contour_sorted;

	const int i_max = contour_size + index_dist_min;
	for (int i = index_dist_min; i < i_max; ++i)
	{
		const int index = i >= contour_size ? i - contour_size : i;
		contour_sorted.push_back((*contour)[index]);
	}

	double dist_max = 0;
	vector<double> dist_vec;

	for (Point pt : contour_sorted)
	{
		double dist = get_distance(pt, pivot);
		low_pass_filter.compute(dist, 0.5, "dist");

		if (dist > dist_max)
			dist_max = dist;

		dist_vec.push_back(dist);
	}

	vector<int> concave_indexes;

	const int dist_vec_size = dist_vec.size();
	for (int i = 0; i < dist_vec_size; ++i)
	{
		double y = dist_vec[i];
		const int index_begin = i + 1;
		int index_end = 0;

		if (dist_vec[index_begin] >= y)
			continue;

		for (int x = index_begin; x < dist_vec_size; ++x)
			if (dist_vec[x] >= y)
			{
				index_end = x;
				break;
			}
		if (index_end == 0)
			continue;

		double y_min = 9999;
		int index_y_min;

		const int index_end_const = index_end;
		for (int x = index_begin; x < index_end_const; ++x)
		{
			y = dist_vec[x];
			if (y < y_min)
			{
				y_min = y;
				index_y_min = x;
			}
		}
		if (y - y_min > 0)
			concave_indexes.push_back(index_y_min);
	}
	Mat image_palm = Mat::zeros(HEIGHT_SMALL, WIDTH_SMALL, CV_8UC1);

	const int image_width_resized = WIDTH_SMALL / 2;
	const int image_height_resized = HEIGHT_SMALL / 2;

	Mat image_resized;
	resize(image_in, image_resized, Size(image_width_resized, image_height_resized), 0, 0, INTER_LINEAR);
	
	Mat image_distance_transform;
	distanceTransform(image_resized, image_distance_transform, DIST_L2, DIST_MASK_PRECISE);

	double min;
	double max;
	Point min_loc;
	Point max_loc;
	minMaxLoc(image_distance_transform, &min, &max, &min_loc, &max_loc);

	Point palm_circle_center = max_loc * 2;
	const double palm_circle_radius = max * 2;
	const double palm_circle_max_y = palm_circle_center.y + palm_circle_radius;

	//rotated rect masking of palm
	//mark
	double theta = atan2(palm_circle_center.y - pivot.y, palm_circle_center.x - pivot.x) * 180 / CV_PI;
	low_pass_filter.compute(theta, 0.5, "theta");

	RotatedRect r_rect = RotatedRect(palm_circle_center, Size2f(100, 50), theta);

	Point2f vertices[4];
	r_rect.points(vertices);

	const double center_x = (vertices[2].x + vertices[3].x) / 2;
	const double center_y = (vertices[2].y + vertices[3].y) / 2;
	const double x_diff_double = (center_x - palm_circle_center.x) * 1.2;
	const double y_diff_double = (center_y - palm_circle_center.y) * 1.2;

	Mat image_mask = Mat::zeros(HEIGHT_SMALL, WIDTH_SMALL, CV_8UC1);

	for (int i = 0; i < 4; ++i)
	{
		const double x0 = vertices[i].x - x_diff_double;
		const double y0 = vertices[i].y - y_diff_double;
		const double x1 = vertices[(i + 1) % 4].x - x_diff_double;
		const double y1 = vertices[(i + 1) % 4].y - y_diff_double;
		line(image_mask, Point(x0, y0), Point(x1, y1), Scalar(254), 1);
	}
	floodFill(image_mask, Point(WIDTH_SMALL / 2, HEIGHT_SMALL - 1), Scalar(254));
	for (int i = 0; i < WIDTH_SMALL; ++i)
		for (int j = 0; j < HEIGHT_SMALL; ++j)
			if (image_mask.ptr<uchar>(j, i)[0] == 0)
				image_palm.ptr<uchar>(j, i)[0] = 63;
	//

	if (concave_indexes.size() > 0)
	{
		vector<Point> concave_points;

		const int concave_indexes_size = concave_indexes.size();
		for (int i = 0; i < concave_indexes_size; ++i)
			concave_points.push_back(contour_sorted[concave_indexes[i]]);

		for (int i = 1; i < concave_indexes_size; ++i)
		{
			Point* pt_new = &concave_points[i];
	    	Point* pt_old = &concave_points[i - 1];
			line(image_palm, *pt_old, *pt_new, Scalar(254), 1);
		}
		const int extension_num = 200;
		Point concave_point_first = concave_points[0];
		Point concave_point_last = concave_points[concave_indexes_size - 1];
		concave_point_first.x -= extension_num / 2;
		concave_point_last.x += extension_num / 2;
		concave_point_first.y -= extension_num;
		concave_point_last.y -= extension_num;

		line(image_palm, concave_points[0], concave_point_first, Scalar(254), 1);
		line(image_palm, concave_points[concave_indexes_size - 1], concave_point_last, Scalar(254), 1);
	}
	floodFill(image_palm, Point(0, HEIGHT_SMALL - 1), Scalar(127));
	circle(image_palm, palm_circle_center, palm_circle_radius, Scalar(254), -1);
	rectangle(image_palm, Rect(palm_circle_center.x - (max * 2), 0, palm_circle_radius * 2, palm_circle_center.y), Scalar(200), -1);

	Mat image_palm_find_contours = Mat::zeros(HEIGHT_SMALL, WIDTH_SMALL, CV_8UC1);

	for (int i = 0; i < WIDTH_SMALL; ++i)
		for (int j = 0; j < HEIGHT_SMALL; ++j)
			if (image_palm.ptr<uchar>(j, i)[0] != 127 && image_in.ptr<uchar>(j, i)[0] == 254)
			{
				image_in.ptr<uchar>(j, i)[0] = 127;
				image_palm_find_contours.ptr<uchar>(j, i)[0] = 254;
			}

	blob_detector1.compute(image_in, 254);
	for (int i = 0; i < WIDTH_SMALL; ++i)
		for (int j = 0; j < HEIGHT_SMALL; ++j)
			if (image_thinning.ptr<uchar>(j, i)[0] > 0 && image_in.ptr<uchar>(j, i)[0] == 254)
				image_in.ptr<uchar>(j, i)[0] = 63;

	blob_detector0.compute(image_in, 63);
	blob_detector0.sort_blobs_by_count();

	vector<BlobNew> skeleton_parts;
	int width_proper_lowpassed;
	int count_lowpassed;

	for (BlobNew& blob_skeleton_raw : blob_detector0.blobs)
	{
		if (blob_skeleton_raw.count < 5)
			continue;

		double dist_min = 9999;
		Point* pt_dist_min = NULL;

		for (Point& pt : blob_skeleton_raw.data)
		{
			double dist = get_distance(pt, pivot);
			if (pt.x > 0 && pt.y > 0 && pt.x < WIDTH_SMALL - 1 && pt.y < HEIGHT_SMALL - 1)
				if (dist < dist_min)
				{
					dist_min = dist;
					pt_dist_min = &pt;
				}
		}
		blob_detector2.compute_location(image_in, 63, pt_dist_min->x, pt_dist_min->y);
		BlobNew* blob_skeleton_sorted = blob_detector2.blob_max_size;

		const int blob_data_size = blob_skeleton_sorted->data.size();

		Point* pt_new = &(blob_skeleton_sorted->data[blob_data_size - 1]);
		Point* pt_old;

		if (blob_data_size >= 10)
			pt_old = &(blob_skeleton_sorted->data[blob_data_size - (blob_data_size / 2)]);
		else
			pt_old = &(blob_skeleton_sorted->data[blob_data_size - (blob_data_size / 1.25)]);

		vector<Point> line_points;

		if (pt_new->y > pt_old->y)
		{
			Point pt_intersection = get_intersection_at_y(*pt_old, *pt_new, HEIGHT_SMALL);
			bresenham(pt_new->x, pt_new->y, pt_intersection.x, pt_intersection.y, line_points, 20);
		}
		else if (pt_new->y < pt_old->y)
		{
			Point pt_intersection = get_intersection_at_y(*pt_old, *pt_new, 0);
			bresenham(pt_new->x, pt_new->y, pt_intersection.x, pt_intersection.y, line_points, 20);
		}
		else if (pt_new->y == pt_old->y)
		{
			Point pt_intersection = Point(0, pt_new->y);
			if (pt_new->x < pt_old->x)
				pt_intersection.x = pt_new->x - 20;
			else
				pt_intersection.x = pt_new->x + 20;

			bresenham(pt_new->x, pt_new->y, pt_intersection.x, pt_intersection.y, line_points, 20);
		}

		for (Point& pt : line_points)
		{
			uchar pix_val = image_in.ptr<uchar>(pt.y, pt.x)[0];
			if (pix_val == 0)
			{
				int x_diff = pt_new->x - pt_old->x;
				int y_diff = pt_new->y - pt_old->y;

				if (x_diff < 0 || y_diff < 0)
				{
					x_diff = abs(x_diff);
					y_diff = abs(y_diff);
				}
				else
					x_diff = -x_diff;

				vector<Point> line_points1;
				vector<Point> line_points2;
				vector<Point> line_points3;
				vector<Point> line_points4;

				int x_adjusted = pt_old->x + y_diff;
				int y_adjusted = pt_old->y + x_diff;
				if (y_adjusted == pt_old->y)
					y_adjusted -= 1;
				Point pt_adjusted(x_adjusted, y_adjusted);

				Point pt_intersection = get_intersection_at_y(*pt_old, pt_adjusted, 0);
				bresenham(pt_old->x, pt_old->y, pt_intersection.x, pt_intersection.y, line_points1, 10);
				blob_skeleton_raw.pt_intersection0 = pt_intersection;
				
				pt_intersection	 = get_intersection_at_y(*pt_old, pt_adjusted, HEIGHT_SMALL);
				bresenham(pt_old->x, pt_old->y, pt_intersection.x, pt_intersection.y, line_points2, 10);
				blob_skeleton_raw.pt_intersection1 = pt_intersection;

				x_adjusted = pt_new->x + y_diff;
				y_adjusted = pt_new->y + x_diff;
				if (y_adjusted == pt_new->y)
					y_adjusted -= 1;
				pt_adjusted = Point(x_adjusted, y_adjusted);

				pt_intersection = get_intersection_at_y(*pt_new, pt_adjusted, 0);
				bresenham(pt_new->x, pt_new->y, pt_intersection.x, pt_intersection.y, line_points3, 10);
				blob_skeleton_raw.pt_intersection2 = pt_intersection;

				pt_intersection = get_intersection_at_y(*pt_new, pt_adjusted, HEIGHT_SMALL);
				bresenham(pt_new->x, pt_new->y, pt_intersection.x, pt_intersection.y, line_points4, 10);
				blob_skeleton_raw.pt_intersection3 = pt_intersection;

				int white_pix_count0(0);
				int white_pix_count1(0);
				int white_pix_count2(0);
				int white_pix_count3(0);

				for (Point& pt_line : line_points1)
				{
					if (image_in.ptr<uchar>(pt_line.y, pt_line.x)[0] == 0)
						break;
					++white_pix_count0;
				}
				for (Point& pt_line : line_points2)
				{
					if (image_in.ptr<uchar>(pt_line.y, pt_line.x)[0] == 0)
						break;
					++white_pix_count1;
				}
				for (Point& pt_line : line_points3)
				{
					if (image_in.ptr<uchar>(pt_line.y, pt_line.x)[0] == 0)
						break;
					++white_pix_count2;
				}
				for (Point& pt_line : line_points4)
				{
					if (image_in.ptr<uchar>(pt_line.y, pt_line.x)[0] == 0)
						break;
					++white_pix_count3;
				}
				blob_skeleton_raw.width_proper = white_pix_count0 + white_pix_count1 + white_pix_count2 + white_pix_count3;
				blob_skeleton_raw.x = pt.x;
				blob_skeleton_raw.y = pt.y;
				blob_skeleton_raw.x0 = pt_old->x;
				blob_skeleton_raw.y0 = pt_old->y;
				blob_skeleton_raw.dist = get_distance(pivot, pt);

				skeleton_parts.push_back(blob_skeleton_raw);

				width_proper_lowpassed = blob_skeleton_raw.width_proper;
				low_pass_filter.compute(width_proper_lowpassed, 0.1, "width_proper_lowpassed");

				count_lowpassed = blob_skeleton_sorted->count;
				low_pass_filter.compute(count_lowpassed, 0.1, "count_lowpassed");

				break;
			}
			else if (pix_val == 127)
				break;
		}
	}
	for (BlobNew& blob_skeleton : skeleton_parts)
	{
		if (blob_skeleton.count < count_lowpassed * 0.5)
			if (blob_skeleton.width_proper > width_proper_lowpassed * 1.25)
				continue;

		if (blob_skeleton.dist < 15)
			continue;

		for (BlobNew& blob_fingertip : blob_detector1.blobs)
		{
			const int overlap_count = blob_skeleton.compute_overlap(blob_fingertip, 0, 0);

			if (overlap_count > blob_skeleton.count / 2)
			{
				if (blob_fingertip.matching_blob == NULL)
					blob_fingertip.matching_blob = &blob_skeleton;
				else
				{
					const int x_temp = WIDTH_SMALL / 2;
					const double dist0 = get_distance(blob_skeleton.x, blob_skeleton.y, x_temp, HEIGHT_SMALL);
					const double dist1 = get_distance(blob_fingertip.matching_blob->x, blob_fingertip.matching_blob->y,
													  x_temp, HEIGHT_SMALL);

					if (dist0 < dist1)
						blob_fingertip.matching_blob = &blob_skeleton;
				}
			}
		}
	}
	vector<BlobNew> fingertip_blobs;
	Mat image_fingers = Mat::zeros(HEIGHT_SMALL, WIDTH_SMALL, CV_8UC1);

	double dist_min = 9999;
	Point pt_cursor;

	for (BlobNew& blob_fingertip : blob_detector1.blobs)
		if (blob_fingertip.matching_blob != NULL)
		{
			fingertip_blobs.push_back(blob_fingertip);
			blob_fingertip.matching_blob->fill(image_fingers, 254);

//ad hoc code temporarily used to parse 1 finger gesture

			double dist = get_distance(blob_fingertip.matching_blob->x, blob_fingertip.matching_blob->y,
									   WIDTH_SMALL / 2, HEIGHT_SMALL);

			if (dist < dist_min)
			{
				dist_min = dist;
				pt_cursor.x = blob_fingertip.matching_blob->x;
				pt_cursor.y = blob_fingertip.matching_blob->y;
			}
		}
	// parse_gesture(image_in, fingertip_blobs, white_pix_count_total, skeleton_parts, image_palm_find_contours);
	// imshow("image_fingers", image_fingers);
	imshow("image_fingers", image_fingers);
}

void MonoProcessorNew::bresenham(int x1_in, int y1_in, int const x2_in, int const y2_in, vector<Point>& result_out, const uchar count_in)
{
    int delta_x(x2_in - x1_in);
    signed char const ix((delta_x > 0) - (delta_x < 0));
    delta_x = std::abs(delta_x) << 1;
 
    int delta_y(y2_in - y1_in);
    signed char const iy((delta_y > 0) - (delta_y < 0));
    delta_y = std::abs(delta_y) << 1;
 
    result_out.push_back(Point(x1_in, y1_in));
 
    if (delta_x >= delta_y)
    {
        int error(delta_y - (delta_x >> 1));
 
        while (x1_in != x2_in)
        {
            if ((error >= 0) && (error || (ix > 0)))
            {
                error -= delta_x;
                y1_in += iy;
            }
            error += delta_y;
            x1_in += ix;
 			
 			result_out.push_back(Point(x1_in, y1_in));
    		if (result_out.size() == count_in || x1_in == 0 || y1_in == 0 || x1_in == WIDTH_SMALL_MINUS || y1_in == HEIGHT_SMALL_MINUS)
    			return;
        }
    }
    else
    {
        int error(delta_x - (delta_y >> 1));
 
        while (y1_in != y2_in)
        {
            if ((error >= 0) && (error || (iy > 0)))
            {
                error -= delta_y;
                x1_in += ix;
            }
            error += delta_x;
            y1_in += iy;
 
 			result_out.push_back(Point(x1_in, y1_in));
    		if (result_out.size() == count_in || x1_in == 0 || y1_in == 0 || x1_in == WIDTH_SMALL_MINUS || y1_in == HEIGHT_SMALL_MINUS)
    			return;
        }
    }
}

void MonoProcessorNew::parse_gesture(Mat& image_in, vector<BlobNew>& fingertip_blobs, const int white_pix_count_total,
									 vector<BlobNew>& skeleton_parts, Mat& image_palm_find_contours)
{
	bool result = false;
	BlobNew* blob_candidate = NULL;

	if (fingertip_blobs.size() == 1)
		blob_candidate = &fingertip_blobs[0];

	else if (fingertip_blobs.size() == 2 || fingertip_blobs.size() == 3)
	{
		sort(fingertip_blobs.begin(), fingertip_blobs.end(), compare_matching_blob_y());
		if (fingertip_blobs[0].matching_blob->y - fingertip_blobs[1].matching_blob->y > 10)
			blob_candidate = &fingertip_blobs[0];
	}

	if (blob_candidate != NULL && white_pix_count_total < 200)
		blob_candidate = NULL;

	if (blob_candidate != NULL && blob_candidate->matching_blob != NULL)
		if (blob_candidate->matching_blob->count > 30)
			blob_candidate = NULL;

	if (blob_candidate != NULL)
	{
		vector<vector<Point>> image_palm_contours;
		vector<Vec4i> hiearchy;
		findContours(image_palm_find_contours, image_palm_contours, hiearchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE, Point(0, 0));

		double dist_max = 0;
		for (BlobNew& blob : skeleton_parts)
		{
			Point pt0(blob.x_min, blob.y_max);

			for (vector<Point>& contour : image_palm_contours)
				for (Point& pt1 : contour)
				{
					double dist = get_distance(pt0, pt1);
					if (dist > dist_max)
						dist_max = dist;
				}
		}
		if (dist_max < 30)
			blob_candidate = NULL;

		if (blob_candidate != NULL)
		{
			int white_pix_count = 0;

			vector<Point> line_points0;
			vector<Point> line_points1;

			const int x = blob_candidate->matching_blob->x;
			const int y = blob_candidate->matching_blob->y;
			const int x0 = blob_candidate->matching_blob->pt_intersection0.x;
			const int y0 = blob_candidate->matching_blob->pt_intersection0.y;
			const int x1 = blob_candidate->matching_blob->pt_intersection1.x;
			const int y1 = blob_candidate->matching_blob->pt_intersection1.y;

			bresenham(x, y, x0, y0, line_points0, 100);
			bresenham(x, y, x1, y1, line_points1, 100);

			for (Point& pt : line_points0)
				if (image_in.ptr<uchar>(pt.y, pt.x)[0] == 254)
					++white_pix_count;

			for (Point& pt : line_points1)
				if (image_in.ptr<uchar>(pt.y, pt.x)[0] == 254)
					++white_pix_count;

			if (white_pix_count > 15)
				blob_candidate = NULL;
		}

		static int non_null_count = 0;
		static int null_count = 0;
		static bool non_null = false;

		if (blob_candidate != NULL)
		{

			++non_null_count;
			null_count = 0;

			if (non_null_count > 5)
				non_null = true;
		}
		else if (non_null)
		{
			if (null_count > 3)
			{
				non_null_count = 0;
				non_null = false;
			}
			++null_count;
		}
		if (non_null)
			result = true;
	}
	cout << result << endl;
}