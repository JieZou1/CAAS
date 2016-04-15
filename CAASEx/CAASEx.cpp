#include "CAASCLR4Tx.h"

void caasCLR4TxInspect(const caasInputImage* image, caasInspectResult* result)
{
	std::cout << "From inside caasCLR4TxDetect" << std::endl;


	caasCLR4Tx tx(image); //some general initialization
	tx.Inspect();


	tx.GetResult(result);
}

