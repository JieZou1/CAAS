#include "caasPSM4TxBase.h"

caasPSM4TxBase::caasPSM4TxBase(const caasInput* input) : caasBase(input)
{
}

void caasPSM4TxBase::GetResult(caasPSM4TxOutput* result)
{
	//Filling result struct.
	result->apertureMiddleY = (apertureTopEdge + apertureBottomEdge) / 2;

	//After the result is filled, we calculate processing time.
	endT = std::clock();	result->processingTime = (endT - startT) / (double)CLOCKS_PER_SEC;
}
