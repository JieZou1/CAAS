#include "caasPSM4TxBase.h"

caasPSM4TxBase::caasPSM4TxBase(const caasInput* input) : caasBase(input)
{
}

void caasPSM4TxBase::GetResult(caasOutput* result)
{
	//Filling result struct.
	result->targetRightEdge = targetRightEdge;
	result->targetLeftEdge = targetLeftEdge;
	result->isolatorRightEdge = isolatorRightEdge;

	result->distanceInPixels = targetLeftEdge - isolatorRightEdge;
	result->distanceInMicrons = result->distanceInPixels / pixelsPerMicron;

	result->isolatorAngle = isolatorAngle;

	//After the result is filled, we calculate processing time.
	endT = std::clock();	result->processingTime = (endT - startT) / (double)CLOCKS_PER_SEC;
}
