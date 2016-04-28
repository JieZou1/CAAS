#pragma once

#include "CAASBase.h"

class caasCLR4Tx : public caasBase
{
private:
	const int TARGET_WIDTH_MICRON = 460; //in Micron, 
	int targetWidth;	//In pixels
	int isolatorWidth;	//In pixels
	int isolatorHeight;	//In pixels

	int targetLeftEdge;	//In pixels
	int targetRightEdge;	//In pixels

	int targetTopEdge;	//In pixels
	int targetBottomEdge;	//In pixels

	int isolatorRightEdge;
	int isolatorTopEdge;
	int isolatorBottomEdge;

	double isolatorAngle;

	void FindTargetRightEdge();
	void FindTargetLeftEdge();
	void FindTargetTopBottomEdges();
	void FindIsolator();
	void FindIsolatorAngle();

public:
	caasCLR4Tx(const caasInput* input);
	void Inspect();
	void GetResult(caasOutput* output);
};