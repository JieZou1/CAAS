#include "CAASEx.h"
#include "CAASCLR4Tx.h"

void caasBayer2BGR(caasImage* in, caasImage* out)
{
	cv::Mat imageInput = Mat(in->height, in->width, CV_8UC3, in->data);
	cv::Mat imageOutput;
	cvtColor(imageInput, imageOutput, CV_BayerBG2BGR);

	out->width = in->width;
	out->height = in->height;
	out->type = BGR;
	//out->data = imageOutput.
}

void caasCLR4TxInspect(const caasImage* image, caasInspectResult* result)
{
	std::cout << "From inside caasCLR4TxDetect" << std::endl;


	caasCLR4Tx tx(image); //some general initialization
	tx.Inspect();


	tx.GetResult(result);
}

