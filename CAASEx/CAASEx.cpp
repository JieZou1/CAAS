#include "CAASEx.h"
#include "CAASCLR4Tx.h"

void caasCLR4TxInspect(const caasInput* input, caasOutput* output)
{
	std::cout << "From inside caasCLR4TxDetect" << std::endl;


	caasCLR4Tx tx(input); //some general initialization
	tx.Inspect();


	tx.GetResult(output);
}

