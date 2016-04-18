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
	else if (input->imgType == BayerBGGR12)
	{
		cv::Mat imageBayer16(input->imgHeight, input->imgWidth, CV_16UC1, input->imgData);
		imageBayer = imageBayer16.clone();
		imageBayer.convertTo(imageBayer, CV_8UC1, 0.0625);

		cvtColor(imageBayer, imageGray, COLOR_BayerBG2GRAY);
	}

	imwrite("gray.jpg", imageGray);

}

void caasBase::DetectLineSegments()
{
	lsd = createLineSegmentDetector();
	//lsd = createLineSegmentDetector(LSD_REFINE_ADV);
	lsd->detect(this->imageGray, this->lsdLines);
}
