#include "caasEx.h"
#include "caasCLR4Tx1.h"
#include "caasCLR4TxHOG.h"
#include "caasPSM4TxHOG.h"

int handleError(int status, const char* func_name,
	const char* err_msg, const char* file_name,
	int line, void* userdata)
{
	//Do nothing -- will suppress console output
	return 0;   //Return value is not used
}

void caasCLR4TxInspect(const caasInput* input, caasCLR4TxOutput* output)
{
	cv::redirectError(handleError); //Let's always bypass OpenCV error message console output

	output->targetLeftEdge = output->targetRightEdge = output->isolatorRightEdge = -1;

	//caasBase* inspector = NULL;	//bool error = false;
	try
	{
		//inspector = new caasCLR4TxHOG(input); 
		//caasCLR4Tx1 clr4tx(input); //some general initialization
		caasCLR4TxHOG clr4tx(input); //some general initialization
		clr4tx.Inspect();
		clr4tx.GetResult(output);
		//Check errors
		//if (output->targetLeftEdge == -1 || output->targetRightEdge == -1 || output->isolatorRightEdge == -1) error = true;
	}
	catch (const char* msg)
	{
		cout << msg << endl;
	}
	catch (Exception& e)
	{
		cout << e.what() << endl;
	}
	catch (...)
	{
		cout << "Unknown Error!" << endl;
	}

	//if (inspector != NULL)
	//{
	//	//if (error)	tx->SaveGrayImage();
	//	delete inspector;
	//}
}
void caasPSM4TxInspect(const caasInput* input, caasPSM4TxOutput* output)
{
	cv::redirectError(handleError); //Let's always bypass OpenCV error message console output

	output->arrayblockRightEdge = output->isolatorLeftEdge = output->apertureMiddleY = output->arrayblockMiddleY = -1;

	//caasBase* inspector = NULL;	//bool error = false;
	try
	{
		caasPSM4TxHOG psm4x(input);
		psm4x.Inspect();
		psm4x.GetResult(output);
		//Check errors
		//if (output->targetLeftEdge == -1 || output->targetRightEdge == -1 || output->isolatorRightEdge == -1) error = true;
	}
	catch (const char* msg)
	{
		cout << msg << endl;
	}
	catch (Exception& e )
	{
		cout << e.what() << endl;
	}
	catch (...)
	{
		cout << "Unknown Error!" << endl;
	}

	//if (inspector != NULL)
	//{
	//	//if (error)	tx->SaveGrayImage();
	//	delete inspector;
	//}
}


