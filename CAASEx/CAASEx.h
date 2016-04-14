#pragma once

#include <cv.hpp>

#include <iostream>

using namespace cv;
using namespace std;

//data structure to hold inspection result of an image
struct caasInspectResult
{

};

//Detect function
void caasCLR4TxDetect(const cv::Mat& image, caasInspectResult& result);

class caasBase
{
protected:
	Mat imageColor;	//The original color image
	Mat imageGray;	//The grayscale image
	caasInspectResult* result;	//The Detect Result

	caasBase(Mat image_color, caasInspectResult* pResult);
	void DetectLineSegments();

public:
	virtual void Inspect() = 0;
};

class caasCLR4Tx : public caasBase
{
	caasCLR4Tx(Mat image_color, caasInspectResult* pResult);
};