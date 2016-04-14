#include "CAASEx.h"
#include "CAASCLR4Tx.h"

void caasCLR4TxInspect(const cv::Mat& image, caasInspectResult* result)
{
	std::cout << "From inside caasCLR4TxDetect" << std::endl;

	//Set global parameters
	caasCLR4Tx tx(image, result);
	tx.Inspect();
}

caasCLR4Tx::caasCLR4Tx(Mat image_color, caasInspectResult* pResult) : caasBase(image_color, pResult)
{
}

void caasCLR4Tx::Inspect()
{
	DetectLineSegments();

	lsd->drawSegments(imageColor, lsdLines);
	imwrite("result.jpg", imageColor);
}