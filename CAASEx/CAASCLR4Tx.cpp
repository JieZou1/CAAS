#include "CAASCLR4Tx.h"

caasCLR4Tx::caasCLR4Tx(const caasInput* input) : caasBase(input)
{
}

void caasCLR4Tx::Inspect()
{
	DetectLineSegments();

}

void caasCLR4Tx::GetResult(caasOutput* result)
{
	//TODO filling result struct.

	//After the result is filled, we calculate processing time.
	endT = std::clock();	result->processingTime = (endT - startT) / (double)CLOCKS_PER_SEC;

	lsd->drawSegments(imageGray, lsdLines);
	imwrite("result.jpg", imageGray);
}
