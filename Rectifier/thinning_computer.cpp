#include "thinning_computer.h"

void ThinningComputer::thinningIteration(cv::Mat& img, const int iter)
{
    const int x_min_const = x_min;
    const int x_max_const = x_max;
    const int y_min_const = y_min;
    const int y_max_const = y_max;

    x_min = 9999;
    x_max = 0;
    y_min = 9999;
    y_max = 0;

    Mat marker = cv::Mat::zeros(img.size(), CV_8UC1);

    // initialize row pointers
    uchar* pCurr = img.ptr<uchar>(0);
    uchar* pBelow = img.ptr<uchar>(1);

    for (int y = y_min_const; y <= y_max_const; ++y)
    {
        // shift the rows up by one
        uchar* pAbove = pCurr;
        pCurr = pBelow;
        pBelow = img.ptr<uchar>(y+1);

        uchar* pDst = marker.ptr<uchar>(y);

        // initialize col pointers
        uchar* no = &(pAbove[0]);
        uchar* ne = &(pAbove[1]);
        uchar* me = &(pCurr[0]);
        uchar* ea = &(pCurr[1]);
        uchar* so = &(pBelow[0]);
        uchar* se = &(pBelow[1]);

        for (int x = x_min_const; x <= x_max_const; ++x)
        {
            // shift col pointers left by one (scan left to right)
            uchar* nw = no;
            no = ne;
            ne = &(pAbove[x+1]);

            uchar* we = me;
            me = ea;
            ea = &(pCurr[x+1]);

            uchar* sw = so;
            so = se;
            se = &(pBelow[x+1]);

            int A  = (*no == 0 && *ne == 1) + (*ne == 0 && *ea == 1) + (*ea == 0 && *se == 1) + (*se == 0 && *so == 1) + 
                     (*so == 0 && *sw == 1) + (*sw == 0 && *we == 1) + (*we == 0 && *nw == 1) + (*nw == 0 && *no == 1);

            int B  = *no + *ne + *ea + *se + *so + *sw + *we + *nw;
            int m1 = iter == 0 ? (*no * *ea * *so) : (*no * *ea * *we);
            int m2 = iter == 0 ? (*ea * *so * *we) : (*no * *so * *we);

            if (A == 1 && (B >= 2 && B <= 6) && m1 == 0 && m2 == 0)
            {
                pDst[x] = 1;

                if (x < x_min)
                    x_min = x;
                if (x > x_max)
                    x_max = x;
                if (y < y_min)
                    y_min = y;
                if (y > y_max)
                    y_max = y;
            }
        }
    }
	if (x_min == 9999)
		x_min = x_min_const;
    else
    {
        x_min -= 10;
        if (x_min < 1)
            x_min = 1;
    }
	if (x_max == 0)
		x_max = x_max_const;
    else
    {
        x_max += 10;
        if (x_max > img.cols - 2)
            x_max = img.cols - 2;
    }
	if (y_min == 9999)
		y_min = y_min_const;
    else
    {
        y_min -= 10;
        if (y_min < 1)
            y_min = 1;
    }
	if (y_max == 0)
		y_max = y_max_const;
    else
    {
        y_max += 10;
        if (y_max > img.rows - 2)
            y_max = img.rows - 2;
    }

    img &= ~marker;
}

void ThinningComputer::thinning(cv::Mat& src)
{
    x_min = 1;
    x_max = src.cols - 2;
    y_min = 1;
    y_max = src.rows - 2;

	src /= 254;
    cv::Mat prev = cv::Mat::zeros(src.size(), CV_8UC1);
    cv::Mat diff;

    do
    {
        thinningIteration(src, 0);
        thinningIteration(src, 1);
        cv::absdiff(src, prev, diff);
        src.copyTo(prev);
    } 
    while (cv::countNonZero(diff) > 0);
    src *= 254;
}