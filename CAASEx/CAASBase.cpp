#include "CAASBase.h"

caasBase::caasBase(const caasInput* input)
{
	startT = std::clock();	//The inspection starts here

	//Convert to gray image
	if (input->imgType == BGR)
	{
		Mat imageColor(input->imgHeight, input->imgWidth, CV_8UC3, input->imgData);
		cvtColor(imageColor, imageGray, CV_BGR2GRAY);
	}
	else if (input->imgType == BayerBGGR12)
	{
		Mat imageBayer(input->imgHeight, input->imgWidth, CV_16UC1, input->imgData);
		imageBayer.convertTo(imageBayer, CV_8UC1, 0.0625);	//0.0625 is 1/16. Convert orignal 16 bit (actually 12 bit) to 8 bit.
		cvtColor(imageBayer, imageGray, COLOR_BayerBG2GRAY);
	}
}

void caasBase::DetectLineSegments()
{
	lsd = createLineSegmentDetector();
	//lsd = createLineSegmentDetector(LSD_REFINE_ADV);
	lsd->detect(this->imageGray, this->lsdLines);
}
