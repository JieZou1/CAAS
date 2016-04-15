#pragma once

#include "CAASEx.h"
#include <ctime>
#include <cv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

class caasBase
{
protected:
	Mat imageColor;	//The original color image
	Mat imageGray;	//The grayscale image

	clock_t startT, endT;	//For calculating processing time

	Ptr<LineSegmentDetector> lsd;	//The LSD implemented in OpenCV
	vector<Vec4f> lsdLines;			//The lines detected by LSD

	void DetectLineSegments();

public:
	caasBase(const caasInputImage* caasImage);
	virtual void Inspect() = 0;
	virtual void GetResult(caasInspectResult* result) = 0;
};

