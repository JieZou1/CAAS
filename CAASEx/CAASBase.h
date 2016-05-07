#pragma once

#include "CAASEx.h"
#include <ctime>
#include <cv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

class caasBase
{
protected: //Helper functions
	int Median(Mat image); //Find the median grayvalue of the grayscale image
	void ProjectionProfileAnalysis(Mat& verProjection, float& min_value, int& min_index, float& max_value, int& max_index, float values[]);
	void Gradient(int length, float values[], float gradients[]); //Find the gradient of values using [-1 0 1], and save in graidents

protected:
	double pixelsPerMicron;

	clock_t startT, endT;	//For calculating processing time

	Mat imageGray;	//The grayscale image, currently we do inspection on gray scale image

	void DetectLineSegments();

public:
	caasBase(const caasInput* input);
	virtual void Inspect() = 0;
	virtual void GetResult(caasOutput* output) = 0;

	void SaveGrayImage();
};

