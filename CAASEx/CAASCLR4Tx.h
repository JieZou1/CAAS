#pragma once

#include "CAASBase.h"

class caasCLR4Tx : public caasBase
{
private: //All image related fields
	//Mat imageGrayQuarter; //resized to 1/4 of the original size

private: //All measurements related fields
	const int TARGET_WIDTH_MICRON = 460; //in Micron, 
	int targetWidth;	//In pixels
	int isolatorWidth;	//In pixels
	int isolatorHeight;	//In pixels

	int targetLeftEdge;	//In pixels
	int targetRightEdge;	//In pixels
	int targetTopEdge;	//In pixels
	int targetBottomEdge;	//In pixels

	int isolatorLeftEdge;
	int isolatorRightEdge;
	int isolatorTopEdge;
	int isolatorBottomEdge;

	double isolatorAngle;

private: //Methods
	void FindTargetRightEdge();
	void FindTargetLeftEdge();
	void FindTargetTopBottomEdges();
	void FindIsolator();
	void FindIsolatorAngle();
	void RefineIsolator();

public:
	caasCLR4Tx(const caasInput* input);
	void Inspect();
	void GetResult(caasOutput* output);
};