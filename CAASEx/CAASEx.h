#pragma once

#include <cv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

//data structure to hold inspection result of an image
struct caasInspectResult
{
	double processingTime;
};

//Detect function
void caasCLR4TxInspect(const cv::Mat& image, caasInspectResult* result);

