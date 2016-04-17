#include "CAASEx.h"
#include "CAASCLR4Tx.h"

void caasBayer2BGR(caasInput* in, caasInput* out)
{
	cv::Mat imageInput(in->imgHeight, in->imgWidth, CV_8UC1, in->imgData);
	cv::Mat imageOutput(out->imgHeight, out->imgWidth, CV_8UC3, out->imgData);
	cvtColor(imageInput, imageOutput, CV_BayerBG2BGR);

	out->imgWidth = in->imgWidth;
	out->imgHeight = in->imgHeight;
	out->imgType = BGR;
	//out->imageData = imageOutput.
}

void caasCLR4TxInspect(const caasInput* input, caasOutput* output)
{
	std::cout << "From inside caasCLR4TxDetect" << std::endl;


	caasCLR4Tx tx(input); //some general initialization
	tx.Inspect();


	tx.GetResult(output);
}

