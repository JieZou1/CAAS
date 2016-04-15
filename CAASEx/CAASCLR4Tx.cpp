#include "CAASCLR4Tx.h"

caasCLR4Tx::caasCLR4Tx(const caasInputImage* caasimage) : caasBase(caasimage)
{
}

void caasCLR4Tx::Inspect()
{
	DetectLineSegments();

}

void caasCLR4Tx::GetResult(caasInspectResult* result)
{
	//TODO filling result struct.

	//After the result is filled, we calculate processing time.
	endT = std::clock();	result->processingTime = (endT - startT) / (double)CLOCKS_PER_SEC;

	lsd->drawSegments(imageColor, lsdLines);
	imwrite("result.jpg", imageColor);
}
