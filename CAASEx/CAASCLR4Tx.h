#pragma once

#include "CAASBase.h"

class caasCLR4Tx : public caasBase
{
public:
	caasCLR4Tx(const caasInputImage* caasimage);
	void Inspect();
	void GetResult(caasInspectResult* result);
};