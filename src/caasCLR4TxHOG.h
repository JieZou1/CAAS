#pragma once
#include "caasCLR4TxBase.h"

class caasCLR4TxHOG final : public caasCLR4TxBase
{
private:
	Size	TARGET_SIZE_ORIGINAL,	TARGET_SIZE_HOG,
			ISOLATOR_SIZE_ORIGINAL,	ISOLATOR_SIZE_HOG,
			BASE_SIZE_ORIGINAL, BASE_SIZE_HOG;
	float	TARGET_SCALING, ISOLATOR_SCALING, BASE_SCALING;

	HOGDescriptor			targetHOG,				isolatorHOG,	baseHOG;
	std::vector<HoGResult>	targetHOGResults,		isolatorHOGResults, baseHOGResults;
	const int				TARGET_HOG_N = 3457, ISOLATOR_HOG_N = 2917, BASE_HOG_N = 3132;

	bool DrawLineSegments(vector<Vec4f>& lsdLines, Mat& lsdImage);

private:
	void LocateTarget();
	void RefineTarget();
	void LocateIsolator();
	void RefineIsolator();
	void FindIsolatorAngle();

	void LocateBase();
	void RefineBase();

public:
	caasCLR4TxHOG(const caasInput* input);
	void Inspect();
};

