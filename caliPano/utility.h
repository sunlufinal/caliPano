#ifndef __UTILITY__
#define __UTILITY__

#include <opencv2\opencv.hpp>

cv::Mat combine2ImagesHorizontally(cv::Mat img1, cv::Mat img2);

cv::Mat combine2ImagesVertically(cv::Mat img1, cv::Mat img2);

#endif