#include "caasCLR4TxHOG.h"

caasCLR4TxHOG::caasCLR4TxHOG(const caasInput* input) : caasCLR4TxBase(input) {}

void caasCLR4TxHOG::Inspect()
{
#if _DEBUG
	imwrite("0.1.gray.jpg", imageGray);
#endif

	//TODO: use HOG method to locate the target/isolator and then refine the edges of target/isolator
	LocateTarget();

	return;
}

void caasCLR4TxHOG::LocateTarget()
{
	HOGDescriptor hog = HOGDescriptor();
}