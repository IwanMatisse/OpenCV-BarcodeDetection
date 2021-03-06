
#include "stdafx.h"
#include <string>
#include <opencv2\opencv.hpp>
#include "opencv2\core\core.hpp"
#include "opencv2\imgcodecs.hpp"

int main(int argc, char *argv[])
{
	//arguments: source_file result_file
	if (argc != 3) return 0;

	std::string img_file = std::string(argv[1]);
	auto image = cv::imread(img_file, 1);
	
	cv::Mat work_image;
	cv::cvtColor(image, work_image, cv::COLOR_BGR2GRAY);

	//compute the Scharr gradient magnitude representation of the images
	// in both the x and y direction
	cv::Mat gradX = cv::Mat::zeros(image.size(), image.type());;
	cv::Sobel(work_image, gradX, CV_32F, 1, 0, -1);
	cv::Mat gradY = cv::Mat::zeros(image.size(), image.type());;
	cv::Sobel(work_image, gradY, CV_32F, 0, 1, -1);

	// subtract the y - gradient from the x - gradient
	cv::Mat gradient = cv::Mat::zeros(image.size(), image.type());;
	cv::subtract(gradX, gradY, gradient);
	cv::convertScaleAbs(gradient, gradient);

	//blur and threshold the image
	cv::Mat blurred;
	cv::blur(gradient, blurred, cv::Size(9,9));
	
	cv::Mat thresh;
	cv::threshold(blurred, thresh, 225, 255, cv::THRESH_BINARY);
	
	// construct a closing kernel and apply it to the thresholded image
	cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(21, 7));
	cv::Mat closed;
	cv::morphologyEx(thresh, closed, cv::MORPH_CLOSE, kernel);

	// perform a series of erosions and dilations
	cv::erode(closed, closed, cv::Mat(), cv::Point(-1,-1), 4);
	cv::dilate(closed, closed, cv::Mat(), cv::Point(-1, -1), 4);

	// find the contours in the thresholded image, then sort the contours
	// by their area, keeping only the largest one
	cv::Mat copy;
	closed.copyTo(copy);
	std::vector<std::vector<cv::Point>> contours;
	cv::findContours(copy, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

	struct {
		bool operator()(std::vector<cv::Point> &a, std::vector<cv::Point> &b) const
		{
			return cv::contourArea(a) > cv::contourArea(b);
		}
	} comparator;
	std::sort(contours.begin(), contours.end(), comparator);
	
	//compute the rotated bounding box of the largest contour
	auto rect = cv::minAreaRect(contours[0]);	
	cv::Scalar green = cv::Scalar(0.0, 255.0, 0.0);
	
	cv::Point2f vertices2f[4];
	rect.points(vertices2f);
	//draw a bounding box arounded the detected barcode and save the image
	cv::rectangle(image, vertices2f[2], vertices2f[0], green, 5);

	cv::imwrite(std::string(argv[2]), image);
    return 0;
}

