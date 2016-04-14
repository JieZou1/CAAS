#pragma once

#include "CAASBase.h"
#include "CAASCLR4Tx.h"

class caasCLR4Tx : public caasBase
{
public:
	caasCLR4Tx(Mat image_color);
	void Inspect();
	void GetResult(caasInspectResult* result);
};