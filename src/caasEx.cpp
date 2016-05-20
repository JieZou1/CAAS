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

void caasInspect(const PlatformType platform, const caasInput* input, caasOutput* output)
{
	//std::cout << "From inside caasInspect" << std::endl;
	
	cv::redirectError(handleError); //Let's always bypass OpenCV error message console output

	output->targetLeftEdge = output->targetRightEdge = output->isolatorRightEdge = -1;

	//caasBase* inspector = NULL;	//bool error = false;
	try
	{
		switch (platform)
		{
		case CLR4Tx:
		{
			//inspector = new caasCLR4TxHOG(input); 
			//caasCLR4Tx1 clr4tx(input); //some general initialization
			caasCLR4TxHOG clr4tx(input); //some general initialization
			clr4tx.Inspect();
			clr4tx.GetResult(output);
		}
			break;
		case PSM4Tx:
		{
			caasPSM4TxHOG psm4x(input);
			psm4x.Inspect();
			psm4x.GetResult(output);
		}
			break;
		default:
			break;
		}

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

