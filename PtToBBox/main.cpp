#include "stdafx.h"
#include <opencv2/core/core.hpp>

namespace {

cv::Rect blob_box_from_point(cv::Mat image, cv::Point const &pt)
{
    std::vector<cv::Vec4i>              hierarchy;
    std::vector<std::vector<cv::Point>> contours;
	if (image.type() == CV_8U)
        image = image.clone();
    else
        image.convertTo(image, CV_8U);
    findContours(image, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);

    for(int i = 0; i< contours.size(); ++i)
    {
        double const a = contourArea(contours[i], false);

        cv::Rect rc = boundingRect(contours[i]);
        if (pt.x >= rc.x  &&  pt.x <= rc.x + rc.width)
        {
            if (pt.y >= rc.y  &&  pt.y <= rc.y + rc.height)
                return rc;
        }
    }

    return {};
}

cv::Rect bounding_box_from_point(cv::Mat image, cv::Point const &pt)
{
	cv::Mat sal;
	image.convertTo(image, CV_32FC3, 1.0/255);
	sal = CmSaliencyRC::GetRC(image);

	GaussianBlur(sal, sal, Size(9, 9), 0);
	normalize(sal, sal, 0, 1, NORM_MINMAX);

    float pixel = sal.at<float>(pt);
    if (pixel < .25)
    {
        sal = 1.0 - sal;
        pixel = sal.at<float>(pt);
    }
    threshold(sal, sal, pixel-0.01, 1.0, THRESH_TOZERO);
	GaussianBlur(sal, sal, Size(15, 15), 0);
    threshold(sal, sal, 0.0, 1.0, THRESH_BINARY);
    return blob_box_from_point(sal, pt);
}

}   // anonymous namespace

int main(int argc, char* argv[])
{
    std::vector<std::pair<char const *, cv::Point>> samples = {
        { "P43WZd611WA-1.png", {499, 323} },
        { "ncRGoqNUb1w-1.png", {202, 354} },
        { "xcuJ5hYkC-g-1.png", {190, 204} },
        { "bcac7992781fc4e4d739e6109811b4a4_00153.png", {361, 231} },
        { "60a459115fc5e893a54099fdbc638256_00990.png", {228, 177} },
        { "b39b2428e66ff836d01b2d879f32e991_00045.png", {315, 294} },
    };

    for (auto const &sample : samples)
    {
        cv::Mat image = imread(sample.first);
        if (!image.empty())
        {
            cv::Point const &pt = sample.second;
            cv::Rect rc = bounding_box_from_point(image, pt);

            int const size = 4;
            line(image, {pt.x-size,pt.y}, {pt.x+size,pt.y}, CV_RGB(0,255,255));
            line(image, {pt.x,pt.y-size}, {pt.x,pt.y+size}, CV_RGB(0,255,255));
            if (rc.area() > 0)
                rectangle(image, rc, CV_RGB(255,255,0));

            char pathname[_MAX_PATH];
            char drive[_MAX_DRIVE];
            char dir[_MAX_DIR];
            char fname[_MAX_FNAME];
            char ext[_MAX_EXT];
            _splitpath(sample.first, drive, dir, fname, ext);

            strcat(fname,"-result");
            _makepath(pathname, drive, dir, fname, ext);
            imwrite(pathname, image);
        }
    }
}
