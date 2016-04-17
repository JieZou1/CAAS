#pragma once

enum ImageType { BGR, BayerBG8};

//data structure to hold the input image
struct caasInput
{
	//Image Data Inputs
	unsigned char* imgData; 
	int imgWidth;
	int imgHeight;
	ImageType imgType;


};

//data structure to hold inspection result of an image
struct caasOutput
{
	double processingTime;
};

//Conversion function
//void caasBayer2BGR(caasInput* in, caasInput* out);

//Inspection function
void caasCLR4TxInspect(const caasInput* input, caasOutput* output);

