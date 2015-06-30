#include "reprojector.h"

double Reprojector::compute_depth(const int disparity_in)
{
	return ((a_out - d_out) / (1 + pow(disparity_in / c_out, b_out))) + d_out;
}

Point Reprojector::compute_plane_size(double depth)
{
    double width = FOV_WIDTH * (depth / FOV_DEPTH);
    double height = width / 4 * 3;
	return Point(width, height);
}

Point3d Reprojector::reprojectTo3D(double pt0_x, double pt0_y, double pt1_x, double pt1_y)
{
	double disparity_val = abs(pt1_x - pt0_x);
	double depth = compute_depth(disparity_val);

	Point plane_size = compute_plane_size(depth);

	double half_plane_width = plane_size.x / 2;
	double halfPlaneHeight = plane_size.y / 2;

	double real_x = map_val(pt0_x, 0, 640, -half_plane_width,  half_plane_width);
	double real_y = map_val(pt0_y, 0, 480, -halfPlaneHeight,   halfPlaneHeight);

	return Point3d(real_x * 10, real_y * 10, depth * 10);
}

void Reprojector::load(IPC& ipc)
{
	bool serial_first_non_zero = false;
	string serial = "";

	for (int i = 4; i < 10; ++i)
	{
		string str_temp = "";
		str_temp += serial_number[i];

		if (!serial_first_non_zero && str_temp != "0")
			serial_first_non_zero = true;

		if (serial_first_non_zero)
			serial += serial_number[i];
	}

	bool has_complete_calib_data = false;

	if (directory_exists(data_path_current_module))
		if (file_exists(data_path_current_module + "\\0.jpg"))
			if (file_exists(data_path_current_module + "\\1.jpg"))
				if (file_exists(data_path_current_module + "\\stereoCalibData.txt"))
					if (file_exists(data_path_current_module + "\\rect0.txt"))
						if (file_exists(data_path_current_module + "\\rect1.txt"))
							has_complete_calib_data = true;

	if (!has_complete_calib_data)
	{
		first_run = true;

		if (process_running("menu_plus.exe") == 0)
			create_process(menu_file_path, "menu_plus.exe");

		static bool block_thread = true;
		ipc.get_response("menu_plus", "show download", "", [](const string message_body)
		{
			cout << "unblock" << endl;
			block_thread = false;
		});
		
		while (block_thread)
		{
			cout << "blocked" << endl;
			ipc.update();
#ifdef _WIN32
			Sleep(10);
#elif __APPLE__
            usleep(10);
#endif
		}
        create_directory(data_path.c_str());
		//CreateDirectory(data_path.c_str(), NULL);
        create_directory(data_path_current_module.c_str());
	//	CreateDirectory(data_path_current_module.c_str(), NULL);

		copy_file(executable_path + "\\downloader.exe", data_path_current_module + "\\downloader.exe");
		copy_file(executable_path + "\\rectifier.exe", data_path_current_module + "\\rectifier.exe");

		string param0 = "http://d2i9bzz66ghms6.cloudfront.net/data/" + serial + "/0.jpg";
		string param1 = "0.jpg";
		system(("cd " + cmd_quote + data_path_current_module + cmd_quote + "&& downloader.exe -u " + param0 + " -s " + param1).c_str());

		param0 = "http://d2i9bzz66ghms6.cloudfront.net/data/" + serial + "/1.jpg";
		param1 = "1.jpg";
		system(("cd " + cmd_quote + data_path_current_module + cmd_quote + "&& downloader.exe -u " + param0 + " -s " + param1).c_str());

		param0 = "http://d2i9bzz66ghms6.cloudfront.net/data/" + serial + "/stereoCalibData.txt";
		param1 = "stereoCalibData.txt";
		system(("cd " + cmd_quote + data_path_current_module + cmd_quote + "&& downloader.exe -u " + param0 + " -s " + param1).c_str());

		system(("cd " + cmd_quote + data_path_current_module + cmd_quote + "&& rectifier.exe").c_str());

		ipc.send_message("menu_plus", "show next", "");
	}

	ifstream file_stereo_calib_data(data_path_current_module + "\\stereoCalibData.txt");

	bool is_number_new = false;
	bool is_number_old = false;

	int block_count = 0;
	int block[4];

	vector<Point> disparity_data;

	string str_num_temp = "";
	string disparities_string = "";
	
	while (getline(file_stereo_calib_data, disparities_string))
	{
		const int i_max = disparities_string.length();
		for (int i = 0; i < i_max; ++i)
		{
			string str_temp = "";
			str_temp += disparities_string[i];

			if (str_temp != "," && str_temp != ";")
				is_number_new = true;
			else
				is_number_new = false;

			if (is_number_new)
			{
				if (!is_number_old)
					str_num_temp = str_temp;
				else
					str_num_temp += str_temp;
			}
			else if (is_number_old)
			{
				block[block_count] = stoi(str_num_temp);
				++block_count;
			}

			if (block_count == 3)
			{
				bool found = false;

				for (int a = 0; a < disparity_data.size(); ++a)
				{
					if (disparity_data[a].x == block[0])
					{
						found = true;
						disparity_data[a].y = (disparity_data[a].y + abs(block[1] - block[2])) / 2;
					}
					else if (disparity_data[a].y == abs(block[1] - block[2]))
					{
						found = true;
						disparity_data[a].x = min(disparity_data[a].x, block[0]);
					}
				}
				if (!found)
					disparity_data.push_back(Point(block[0], abs(block[1] - block[2])));

				block_count = 0;
			}

			is_number_old = is_number_new;
		}
	}
	sort(disparity_data.begin(), disparity_data.end(), compare_point_x());

	double *t, *y;

	t = new double[disparity_data.size()];
	y = new double[disparity_data.size()];

	for (unsigned int a = 0; a < disparity_data.size(); a++)
	{
		t[a] = (double)(disparity_data[a].y);
		y[a] = (double)(disparity_data[a].x);
	}
	CCurveFitting cf;
	cf.curve_fitting4(t, (int)(disparity_data.size()), y, &a_out, &b_out, &c_out, &d_out);

	delete []t;
	delete []y;

	ifstream file0(data_path_current_module + "\\rect0.txt");
	is_number_new = false;
	is_number_old = false;
	block_count = 0;

	rect_mat0 = new Point*[640];
	for (int i = 0; i < 640; ++i)
		rect_mat0[i] = new Point[480];

	string rect0_string = "";
	while (getline(file0, rect0_string))
	{
		const int i_max = rect0_string.length();
		for (int i = 0; i < i_max; ++i)
		{
			string str_temp = "";
			str_temp += rect0_string[i];

			if (str_temp != " " && str_temp != "," && str_temp != ";")
				is_number_new = true;
			else
				is_number_new = false;

			if (is_number_new)
			{
				if (!is_number_old)
					str_num_temp = str_temp;
				else
					str_num_temp += str_temp;
			}
			else if (is_number_old)
			{
				block[block_count] = stoi(str_num_temp);
				++block_count;
			}
			if (block_count == 4)
			{
				rect_mat0[block[0]][block[1]] = Point(block[2], block[3]);
				block_count = 0;
			}
			is_number_old = is_number_new;
		}
	}
	ifstream file1(data_path_current_module + "\\rect1.txt");
	is_number_new = false;
	is_number_old = false;
	block_count = 0;

	rect_mat1 = new Point*[640];
	for (int i = 0; i < 640; ++i)
		rect_mat1[i] = new Point[480];

	string rect1_string = "";
	while (getline(file1, rect1_string))
	{
		const int i_max = rect1_string.length();
		for (int i = 0; i < i_max; ++i)
		{
			string str_temp = "";
			str_temp += rect1_string[i];

			if (str_temp != " " && str_temp != "," && str_temp != ";")
				is_number_new = true;
			else
				is_number_new = false;

			if (is_number_new)
			{
				if (!is_number_old)
					str_num_temp = str_temp;
				else
					str_num_temp += str_temp;
			}
			else if (is_number_old)
			{
				block[block_count] = stoi(str_num_temp);
				++block_count;
			}
			if (block_count == 4)
			{
				rect_mat1[block[0]][block[1]] = Point(block[2], block[3]);
				block_count = 0;
			}
			is_number_old = is_number_new;
		}
	}
}

Mat Reprojector::remap(Mat* const image_in, const uchar side, const bool interpolate)
{
	const int image_width_const = image_in->cols;
	const int image_height_const = image_in->rows;

	const int scale = 640 / image_in->cols;

	Mat image_out = Mat(image_in->size(), CV_8UC1, Scalar(254));
	Point** rect_mat = NULL;

	if (side == 0)
		rect_mat = rect_mat0;
	else
		rect_mat = rect_mat1;

	Point pt_reprojected;
	uchar gray_reprojected;

	for (int i = 0; i < image_width_const; ++i)
		for (int j = 0; j < image_height_const; ++j)
		{
			pt_reprojected = rect_mat[i * scale][j * scale];
			gray_reprojected = image_in->ptr<uchar>(j, i)[0];

			if (gray_reprojected == 254)
				gray_reprojected = 253;

			image_out.ptr<uchar>(pt_reprojected.y / scale, pt_reprojected.x / scale)[0] = gray_reprojected;
		}

	const int image_width_const_minus = image_width_const - 1;
	const int image_height_const_minus = image_height_const - 1;

	uchar pix0, pix1, pix2, pix3, pix4, pix5, pix6, pix7;
	int pix_mean;
	uchar pix_count;

	if (interpolate)
		for (int i = 1; i < image_width_const_minus; ++i)
			for (int j = 1; j < image_height_const_minus; ++j)
				if (image_out.ptr<uchar>(j, i)[0] == 254)
				{
					pix_mean = 0;
					pix_count = 0;

					pix0 = image_out.ptr<uchar>(j - 1, i - 1)[0];
					pix1 = image_out.ptr<uchar>(j - 1, i)[0];
					pix2 = image_out.ptr<uchar>(j - 1, i + 1)[0];
					pix3 = image_out.ptr<uchar>(j, i - 1)[0];
					pix4 = image_out.ptr<uchar>(j, i + 1)[0];
					pix5 = image_out.ptr<uchar>(j + 1, i - 1)[0];
					pix6 = image_out.ptr<uchar>(j + 1, i)[0];
					pix7 = image_out.ptr<uchar>(j + 1, i + 1)[0];

					if (pix0 == 255 || pix1 == 255 || pix2 == 255 || pix3 == 255 || pix4 == 255 || pix5 == 255 || pix6 == 255 || pix7 == 255)
					{
						continue;
					}
					if (pix0 < 254)
					{
						pix_mean += pix0;
						++pix_count;
					}
					if (pix1 < 254)
					{
						pix_mean += pix1;
						++pix_count;
					}
					if (pix2 < 254)
					{
						pix_mean += pix2;
						++pix_count;
					}
					if (pix3 < 254)
					{
						pix_mean += pix3;
						++pix_count;
					}
					if (pix4 < 254)
					{
						pix_mean += pix4;
						++pix_count;
					}
					if (pix5 < 254)
					{
						pix_mean += pix5;
						++pix_count;
					}
					if (pix6 < 254)
					{
						pix_mean += pix6;
						++pix_count;
					}
					if (pix7 < 254)
					{
						pix_mean += pix7;
						++pix_count;
					}
					pix_mean /= pix_count;
					image_out.ptr<uchar>(j, i)[0] = pix_mean;
				}

	return image_out;
}

Mat Reprojector::compute_gray_image(Mat* const image)
{
	Mat image_gray = Mat(image->size(), CV_8UC1);

	const int image_width_const = image->cols;
	const int image_height_const = image->rows;

	for (int i = 0; i < image_width_const; ++i)
		for (int j = 0; j < image_height_const; ++j)
		{
			int val = max(image->ptr<uchar>(j, i)[0], max(image->ptr<uchar>(j, i)[1], image->ptr<uchar>(j, i)[2]));
			val = pow(val, 2) / 20;
			if (val > 255)
				val = 255;

			image_gray.ptr<uchar>(j, i)[0] = val;
		}

	return image_gray;
}

void Reprojector::compute_interaction_plane(Mat* const image0,               Mat* const image1,
											Mat* const image_chasis_mask0,   Mat* const image_chasis_mask1,
											const int non_reflection_y_max0, const int non_reflection_y_max1)
{
	const int image_width_const = image0->cols;
	const int image_height_const = image0->rows;

	Mat image_gray0 = compute_gray_image(image0);
	Mat image_gray1 = compute_gray_image(image1);

	// for (int i = 0; i < image_width_const; ++i)
	// 	for (int j = 0; j < image_height_const; ++j)
	// {
	// 		if (image_chasis_mask0->ptr<uchar>(j, i)[0] == 0 || j > non_reflection_y_max0)
	// 			image_gray0.ptr<uchar>(j, i)[0] = 0;

	// 		if (image_chasis_mask1->ptr<uchar>(j, i)[0] == 0 || j > non_reflection_y_max1)
	// 			image_gray1.ptr<uchar>(j, i)[0] = 0;
	// 	}

	Mat image_remapped0 = remap(&image_gray0, 0, true);
	Mat image_remapped1 = remap(&image_gray1, 1, true);

	StereoComputer stereo_computer;
	vector<Match*> good_matches = stereo_computer.getMatchingPoints(image_remapped0, image_remapped1, 0, 1);

	Mat image_visualization = Mat::zeros(480, 640, CV_8UC1);

	for (Match* match : good_matches)
	{
		Point3d pt_reprojected = reprojectTo3D(match->img1_pt.x * 4, match->img1_pt.y * 4, match->img2_pt.x * 4, match->img2_pt.y * 4);

		// circle(image_visualization, match->img1_pt, 5, Scalar(254), 1);
		// circle(image_visualization, match->img2_pt, 5, Scalar(127), 1);
		circle(image_visualization, Point((match->img1_pt.x + match->img2_pt.x) / 2, (match->img1_pt.y + match->img2_pt.y) / 2),
			   5, Scalar(pt_reprojected.z / 1000 * 255), 1);
	}

	imshow("image_visualization", image_visualization);
	// imshow("image_remapped0", image_remapped0);
	// imshow("image_remapped1", image_remapped1);
}