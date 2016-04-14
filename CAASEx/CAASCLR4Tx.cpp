#include "CAASEx.h"



void caasCLR4TxDetect(const cv::Mat& image, caasInspectResult& result)
{
	std::cout << "From inside caasCLR4TxDetect" << std::endl;

	//Set global parameters
}

caasCLR4Tx::caasCLR4Tx(Mat image_color, caasInspectResult* pResult) : caasBase(image_color, pResult)
{
}