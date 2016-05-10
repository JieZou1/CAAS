#pragma once
#include "CAASEx.h"
#include "CAASBase.h"

class caasCLR4TxBase : public caasBase
{
protected: //All measurements related fields
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

	float isolatorAngle;

public:
	caasCLR4TxBase(const caasInput* input);
	void GetResult(caasOutput* output);
};

