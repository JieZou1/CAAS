#pragma once

#include "CAASEx.h"
#include <ctime>
#include <cv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

#ifdef _DEBUG
	#define IMWRITE(filename, mat) imwrite(filename, mat)
#else
	#define IMWRITE(filename, mat)
#endif

class caasBase
{
protected: //Helper functions
	int Median(Mat image); //Find the median grayvalue of the grayscale image
	void ProjectionProfileAnalysis(Mat& verProjection, float& min_value, int& min_index, float& max_value, int& max_index, float values[]);
	void Gradient(int length, float values[], float gradients[]); //Find the gradient of values using [-1 0 1], and save in graidents

	void DetectLineSegments(const Mat& image, vector<Vec4f>& lsdLines);

protected:
	double pixelsPerMicron;

	clock_t startT, endT;	//For calculating processing time

	Mat imageGray;	//The grayscale image, currently we do inspection on gray scale image


public:
	caasBase(const caasInput* input);
	virtual void Inspect() = 0;

	void SaveGrayImage();
};

struct HoGResult
{
	double weight;
	cv::Point location;
};

bool SortHoGResultByWeight(const HoGResult &lhs, const HoGResult &rhs);

