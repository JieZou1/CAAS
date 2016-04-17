#pragma once

#include "CAASBase.h"

class caasCLR4Tx : public caasBase
{
public:
	caasCLR4Tx(const caasInput* input);
	void Inspect();
	void GetResult(caasOutput* output);
};