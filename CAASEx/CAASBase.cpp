#include "CAASBase.h"

caasBase::caasBase(Mat image_color)
{
	startT = std::clock();	//The inspection always start here

	imageColor = image_color;
	cvtColor(imageColor, imageGray, CV_BGR2GRAY);
}

void caasBase::DetectLineSegments()
{
	lsd = createLineSegmentDetector();
	lsd->detect(this->imageGray, this->lsdLines);
}
