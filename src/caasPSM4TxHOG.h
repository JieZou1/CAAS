#pragma once
#include "caasPSM4TxBase.h"

class caasPSM4TxHOG final : public caasPSM4TxBase
{
private:
	Size	TARGET_SIZE_ORIGINAL,	TARGET_SIZE_HOG,
			ISOLATOR_SIZE_ORIGINAL,	ISOLATOR_SIZE_HOG;
	float	TARGET_SCALING,			ISOLATOR_SCALING;

	HOGDescriptor			targetHOG,				isolatorHOG;
	std::vector<HoGResult>	targetHOGResults,		isolatorHOGResults;
	const int				TARGET_HOG_N = 3457, ISOLATOR_HOG_N = 2917;

	bool DrawLineSegments(vector<Vec4f>& lsdLines, Mat& lsdImage);

private:
	void LocateTarget();
	void RefineTarget();
	void LocateIsolator();
	void RefineIsolator();
	void FindIsolatorAngle();

public:
	caasPSM4TxHOG(const caasInput* input);
	void Inspect();
};

