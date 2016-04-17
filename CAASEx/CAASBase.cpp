#include "CAASBase.h"

caasBase::caasBase(const caasImage* caasImage)
{
	startT = std::clock();	//The inspection always start here

	imageColor = Mat(caasImage->height, caasImage->width, CV_8UC3, caasImage->data);
	//imwrite("color.jpg", imageColor);
	cvtColor(imageColor, imageGray, CV_BGR2GRAY);
	//imwrite("gray.jpg", imageGray);
}

void caasBase::DetectLineSegments()
{
	lsd = createLineSegmentDetector();
	//lsd = createLineSegmentDetector(LSD_REFINE_ADV);
	lsd->detect(this->imageGray, this->lsdLines);
}
