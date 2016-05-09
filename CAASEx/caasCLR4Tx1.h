#pragma once

#include "CAASCLR4TxBase.h"

class caasCLR4Tx1 : public caasCLR4TxBase
{
private: //All image related fields
	//For detecting right edge of the target
	Mat imageOneTenth;	//The 1/10 size of the original image
	Mat imageOneTenthOtsu; //After otsu thresholding

	//For detecting left edge of the target
	Mat imageOneFourth;
	Mat imageOneFourthMiddleHalf;
	Mat imageOneFourthMiddleHalfCanny;

	//For detecting top and bottom edges of the target
	Mat imageOneFourthTarget;
	Mat imageTarget;	//The final target

	//For detectin isolator edges
	Rect RoiTargetLeftMiddleHalf;
	Mat imageTargetLeftMiddleHalf; //The middel section of the area to the left of the target.
	Mat imageOneFourthTargetLeftMiddleHalf; 
	Mat imageOneFourthTargetLeftMiddleHalfCanny;
	Mat imageIsolatorRoiCanny;
	Mat imageIsolator;

	Mat imageIsolatorRoi;

	//Mat imageGrayQuarter; //resized to 1/4 of the original size

private: //Methods
	void FindTargetRightEdge();
	void FindTargetLeftEdge();
	void FindTargetTopBottomEdges();
	void FindIsolator();
	void FindIsolatorAngle();
	void RefineIsolator();

public:
	caasCLR4Tx1(const caasInput* input);
	void Inspect();
	void GetResult(caasOutput* output);
};