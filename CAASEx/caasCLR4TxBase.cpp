#include "CAASCLR4TxBase.h"


caasCLR4TxBase::caasCLR4TxBase(const caasInput* input) : caasBase(input) 
{
	targetWidth = (int)(input->pixelsPerMicron * TARGET_WIDTH_MICRON);

	isolatorWidth = (int)(1.16 * targetWidth);
	isolatorHeight = (int)(0.87 * targetWidth);

	targetLeftEdge = targetRightEdge = targetTopEdge = targetBottomEdge = -1;
	isolatorLeftEdge = isolatorRightEdge = isolatorTopEdge = isolatorBottomEdge = -1;

	//SaveGrayImage();
}
