#pragma once

#include "CAASBase.h"

class caasCLR4Tx : public caasBase
{
private:
	const int TARGET_WIDTH_MICRON = 460; //in Micron, 
	int targetWidth;	//In pixels
	int metalPosition;	//In pixels

	int FindTargetMetalEdge();
	void FindTargetEdge();

public:
	caasCLR4Tx(const caasInput* input);
	void Inspect();
	void GetResult(caasOutput* output);
};