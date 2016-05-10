#pragma once
#include "caasCLR4TxBase.h"

class caasCLR4TxHOG : public caasCLR4TxBase
{
private:
	void LocateTarget();

public:
	caasCLR4TxHOG(const caasInput* input);
	void Inspect();
};

