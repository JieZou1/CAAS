#include "CAASCLR4TxBase.h"

caasCLR4TxBase::caasCLR4TxBase(const caasInput* input) : caasBase(input) 
{
	targetLeftEdge = targetRightEdge = targetTopEdge = targetBottomEdge = -1;
	isolatorLeftEdge = isolatorRightEdge = isolatorTopEdge = isolatorBottomEdge = -1;

	//SaveGrayImage();
}

void caasCLR4TxBase::GetResult(caasCLR4TxOutput* result)
{
	//Filling result struct.
	result->targetRightEdge = targetRightEdge;
	result->targetLeftEdge = targetLeftEdge;
	result->isolatorRightEdge = isolatorRightEdge;

	result->distanceInPixels = targetLeftEdge - isolatorRightEdge;
	result->distanceInMicrons = result->distanceInPixels / pixelsPerMicron;

	result->isolatorAngle = isolatorAngle;

	result->isolatorMiddleY = (isolatorTopEdge + isolatorBottomEdge) / 2;
	result->baseMiddleY = (baseTopEdge + baseBottomEdge) / 2;
	result->distanceBetweenLinesinMicrons = std::abs(result->isolatorMiddleY-result->baseMiddleY) / pixelsPerMicron;

	//After the result is filled, we calculate processing time.
	endT = std::clock();	result->processingTime = (endT - startT) / (double)CLOCKS_PER_SEC;

}
