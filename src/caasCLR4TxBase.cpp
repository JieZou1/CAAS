#include "CAASCLR4TxBase.h"

caasCLR4TxBase::caasCLR4TxBase(const caasInput* input) : caasBase(input) 
{
	targetLeftEdge = targetRightEdge = targetTopEdge = targetBottomEdge = -1;
	isolatorLeftEdge = isolatorRightEdge = isolatorTopEdge = isolatorBottomEdge = -1;

	//SaveGrayImage();
}

void caasCLR4TxBase::GetResult(caasOutput* result)
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