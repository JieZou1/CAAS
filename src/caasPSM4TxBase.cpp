#include "caasPSM4TxBase.h"

caasPSM4TxBase::caasPSM4TxBase(const caasInput* input) : caasBase(input)
{
}

void caasPSM4TxBase::GetResult(caasPSM4TxOutput* result)
{
	//Filling result struct.
	result->aperture2ndMidY = aperture2ndMidY;
	result->arrayblock1stMidY = arrayblock1stMidY;
	result->isolatorLeftEdge = isolatorLeftEdge;
	result->arrayblockRightEdge = arrayblockRightEdge;

	result->horDistanceInPixels = isolatorLeftEdge - arrayblockRightEdge;
	result->horDistanceInMicrons = result->horDistanceInPixels / pixelsPerMicron;

	result->verDistanceInPixels = abs(arrayblock1stMidY - aperture2ndMidY);
	result->verDistanceInMicrons = result->verDistanceInPixels / pixelsPerMicron;

	result->angle = arrayblockAngle;

	//After the result is filled, we calculate processing time.
	endT = std::clock();	result->processingTime = (endT - startT) / (double)CLOCKS_PER_SEC;
}
