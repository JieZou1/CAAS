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
	double pixelsPerMicron;

	Mat imageGray;	//The grayscale image, currently we do inspection on gray scale image
	Mat imageGrayQuarter; //resized to 1/4 of the original size
	//Mat imageGraySharpened; //After Unsharping masking
	//Mat imageCanny;	//Canny image 

	clock_t startT, endT;	//For calculating processing time

	Ptr<LineSegmentDetector> lsd;	//The LSD implemented in OpenCV
	vector<Vec4f> lsdLines;			//The lines detected by LSD

	int Median(Mat image);

	void DetectLineSegments();

public:
	caasBase(const caasInput* input);
	virtual void Inspect() = 0;
	virtual void GetResult(caasOutput* output) = 0;
};

