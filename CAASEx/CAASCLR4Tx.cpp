#include "CAASEx.h"
#include "CAASCLR4Tx.h"

void caasCLR4TxInspect(const cv::Mat& image, caasInspectResult* result)
{
	std::cout << "From inside caasCLR4TxDetect" << std::endl;


	caasCLR4Tx tx(image); //Complete some general initialization
	tx.Inspect();


	tx.GetResult(result);
}

caasCLR4Tx::caasCLR4Tx(Mat image_color) : caasBase(image_color)
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
