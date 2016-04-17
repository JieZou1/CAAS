#include "CAASBase.h"

caasBase::caasBase(const caasInput* input)
{
	startT = std::clock();	//The inspection always start here

	//Convert to gray image
	if (input->imgType == BGR)
	{
		imageColor = Mat(input->imgHeight, input->imgWidth, CV_8UC3, input->imgData);
		//imwrite("color.jpg", imageColor);
		cvtColor(imageColor, imageGray, CV_BGR2GRAY);
	}
	else if (input->imgType == BayerBG8)
	{
		imageBayer = Mat(input->imgHeight, input->imgWidth, CV_8UC1, input->imgData);
		cvtColor(imageBayer, imageGray, CV_BayerBG2BGR);
	}

	//imwrite("gray.jpg", imageGray);

}

void caasBase::DetectLineSegments()
{
	lsd = createLineSegmentDetector();
	//lsd = createLineSegmentDetector(LSD_REFINE_ADV);
	lsd->detect(this->imageGray, this->lsdLines);
}
