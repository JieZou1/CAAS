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

	int aperture2ndMidY;	//The y coordinate of the middle point of the 2nd aperture from top 
	int arrayblock1stMidY;	//The y coordinate of the middle point of the 1st arrayblock from top 

	float arrayblockAngle;

public:
	caasPSM4TxBase(const caasInput* input);
	void GetResult(caasPSM4TxOutput* output);
};

