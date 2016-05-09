#include "CAASEx.h"
#include "CAASCLR4Tx1.h"

#if !_DEBUG
int handleError(int status, const char* func_name,
	const char* err_msg, const char* file_name,
	int line, void* userdata)
{
	//Do nothing -- will suppress console output
	return 0;   //Return value is not used
}
#endif

void caasCLR4TxInspect(const caasInput* input, caasOutput* output)
{
	//std::cout << "From inside caasCLR4TxDetect" << std::endl;

	output->targetLeftEdge = output->targetRightEdge = output->isolatorRightEdge = -1;
#if !_DEBUG
	cv::redirectError(handleError);
#endif

	caasCLR4Tx1* tx = NULL; bool error = false;
	try
	{
		tx = new caasCLR4Tx1(input); //some general initialization
		tx->Inspect();
		tx->GetResult(output);

		//Check errors
		if (output->targetLeftEdge == -1 || output->targetRightEdge == -1 || output->isolatorRightEdge == -1) error = true;
	}
	catch (...)
	{
		error = true;
	}

	if (tx != NULL)
	{
		if (error)	tx->SaveGrayImage();
		delete tx;
	}
}

