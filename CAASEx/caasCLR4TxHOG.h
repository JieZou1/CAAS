#pragma once
#include "caasCLR4TxBase.h"

class caasCLR4TxHOG final : public caasCLR4TxBase
{
private:
	const int TARGET_ORIGINAL_WIDTH = 500;
	const int TARGET_ORIGINAL_HEIGHT = 2500;
	const int TARGET_HOG_WIDTH = 40;
	const int TARGET_HOG_HEIGHT = 200;
	const int TARGET_HOG_N = 3457;

	const int ISOLATOR_ORIGINAL_WIDTH = 500;
	const int ISOLATOR_ORIGINAL_HEIGHT = 500;
	const int ISOLATOR_HOG_WIDTH = 80;
	const int ISOLATOR_HOG_HEIGHT = 80;
	const int ISOLATOR_HOG_N = 2917;

	bool hogTargetCreated = false;
	bool hogIsolatorCreated = false;

	std::vector<HoGResult> hogResultsTarget;
	std::vector<HoGResult> hogResultsIsolator;

	bool DrawLineSegments(vector<Vec4f>& lsdLines, Mat& lsdImage);

private:
	void LocateTarget();
	void RefineTarget();
	void LocateIsolator();
	void RefineIsolator();
	void FindIsolatorAngle();

public:
	caasCLR4TxHOG(const caasInput* input);
	void Inspect();
};
