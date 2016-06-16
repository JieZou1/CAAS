#pragma once
#include "CAASEx.h"
#include "CAASBase.h"

class caasPSM4TxBase :	public caasBase
{
protected: //All measurements related fields
	int arrayblockLeftEdge;	//In pixels
	int arrayblockRightEdge;	//In pixels
	int arrayblockTopEdge;	//In pixels
	int arrayblockBottomEdge;	//In pixels

	int isolatorLeftEdge;
	int isolatorRightEdge;
	int isolatorTopEdge;
	int isolatorBottomEdge;

	int apertureLeftEdge;
	int apertureRightEdge;
	int apertureTopEdge;
	int apertureBottomEdge;

public:
	caasPSM4TxBase(const caasInput* input);
	void GetResult(caasPSM4TxOutput* output);
};

