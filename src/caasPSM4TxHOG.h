#pragma once
#include "caasPSM4TxBase.h"

class caasPSM4TxHOG final : public caasPSM4TxBase
{
private:
	Size	ARRAYBLOCK_SIZE_ORIGINAL, ARRAYBLOCK_SIZE_HOG,
			ISOLATOR_SIZE_ORIGINAL,	ISOLATOR_SIZE_HOG,
			APERTURE_SIZE_ORIGINAL, APERTURE_SIZE_HOG;
	float	ARRAYBLOCK_SCALING, ISOLATOR_SCALING, APERTURE_SCALING;

	HOGDescriptor			arrayblockHOG,				isolatorHOG,	apertureHOG;
	const int				ARRAYBLOCK_HOG_N = 2052, ISOLATOR_HOG_N = 4860, APERTURE_HOG_N = 1728;
	Rect					arrayblockROI, isolatorROI, apertureROI;	//The HOG search ROI in reduced (small) images.
	std::vector<HoGResult>	arrayblockHOGResults, isolatorHOGResults, apertureHOGResults;

	bool DrawLineSegments(vector<Vec4f>& lsdLines, Mat& lsdImage);

private:
	void LocateAperture();
	void RefineAperture();

	void LocateArrayblock();
	void RefineArrayblock();
	void LocateIsolator();
	void RefineIsolator();
	void FindIsolatorAngle();

public:
	caasPSM4TxHOG(const caasInput* input);
	void Inspect();
};

