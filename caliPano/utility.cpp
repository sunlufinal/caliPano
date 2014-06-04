#include "utility.h"

cv::Mat combine2ImagesHorizontally(cv::Mat img1, cv::Mat img2)
{
	cv::Size size1 = img1.size();
	cv::Size size2 = img2.size();

	cv::Mat com(cv::max(size1.height,size2.height), size1.width+size2.width, img1.type());
	cv::Mat m = cv::Mat::Mat(com, cv::Range(0,size1.height), cv::Range(0,size1.width));
	img1.copyTo(m);
	m = cv::Mat::Mat(com, cv::Range(0,size2.height), cv::Range(size1.width,size1.width+size2.width));
	img2.copyTo(m);

	return com;
}

cv::Mat combine2ImagesVertically(cv::Mat img1, cv::Mat img2)
{
	cv::Size size1 = img1.size();
	cv::Size size2 = img2.size();

	cv::Mat com(size1.height+size2.height, cv::max(size1.width, size2.width), img1.type());
	//cv::Mat com;
	cv::Mat m = cv::Mat::Mat(com, cv::Range(0,size1.height), cv::Range(0,size1.width));
	img1.copyTo(m);
	m = cv::Mat::Mat(com, cv::Range(size1.height, size1.height+size2.height), cv::Range(0, size2.width));
	img2.copyTo(m);

	return com;
}