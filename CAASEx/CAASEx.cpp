#include "CAASEx.h"
#include "CAASCLR4Tx.h"

int handleError(int status, const char* func_name,
	const char* err_msg, const char* file_name,
	int line, void* userdata)
{
	//Do nothing -- will suppress console output
	return 0;   //Return value is not used
}

void caasCLR4TxInspect(const caasInput* input, caasOutput* output)
{
	//std::cout << "From inside caasCLR4TxDetect" << std::endl;

	output->targetLeftEdge = output->targetRightEdge = output->isolatorRightEdge = -1;
#if _DEBUG
	cv::redirectError(handleError);
#endif

	try
	{
		caasCLR4Tx tx(input); //some general initialization
		tx.Inspect();
		tx.GetResult(output);
	}
	catch (...)
	{
	}
}

