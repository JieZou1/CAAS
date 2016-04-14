#include "CAASBase.h"

caasBase::caasBase(Mat image_color, caasInspectResult* pResult)
{
	imageColor = image_color;
	cvtColor(imageColor, imageGray, CV_BGR2GRAY);

	result = pResult;
}

void caasBase::DetectLineSegments()
{
	lsd = createLineSegmentDetector();
	lsd->detect(this->imageGray, this->lsdLines);
}
