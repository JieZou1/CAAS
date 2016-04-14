#pragma once

#include "CAASEx.h"

class caasBase
{
protected:
	Mat imageColor;	//The original color image
	Mat imageGray;	//The grayscale image
	caasInspectResult* result;	//The Detect Result

	Ptr<LineSegmentDetector> lsd;	//The LSD implemented in OpenCV
	vector<Vec4f> lsdLines;			//The lines detected by LSD

	void DetectLineSegments();

public:
	caasBase(Mat image_color, caasInspectResult* pResult);
	virtual void Inspect() = 0;
};

