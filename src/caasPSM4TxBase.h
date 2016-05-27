#pragma once
#include "CAASEx.h"
#include "CAASBase.h"

class caasPSM4TxBase :	public caasBase
{
protected: //All measurements related fields
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
	caasPSM4TxBase(const caasInput* input);
	void GetResult(caasPSM4TxOutput* output);
};

