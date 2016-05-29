#pragma once
#include "CAASEx.h"
#include "CAASBase.h"

class caasCLR4TxBase : public caasBase
{
protected: //All measurements related fields
	int targetLeftEdge;		//In pixels
	int targetRightEdge;	//In pixels
	int targetTopEdge;		//In pixels
	int targetBottomEdge;	//In pixels

	int isolatorLeftEdge;
	int isolatorRightEdge;
	int isolatorTopEdge;
	int isolatorBottomEdge;

	int baseTopEdge;
	int baseBottomEdge;

	float isolatorAngle;

public:
	caasCLR4TxBase(const caasInput* input);
	void GetResult(caasCLR4TxOutput* output);
};

