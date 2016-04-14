#pragma once

#include "CAASEx.h"
#include <ctime>

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
	caasBase(Mat image_color);
	virtual void Inspect() = 0;
	virtual void GetResult(caasInspectResult* result) = 0;
};

