#pragma once

enum ImageType { BGR, BayerBG, BayerGB, BayerGR, BayerRG };

//data structure to hold the input image
struct caasImage
{
	unsigned char* data; //Has to be in BGR 8 bit format
	int width;
	int height;
	ImageType type;
};

//data structure to hold inspection result of an image
struct caasInspectResult
{
	double processingTime;
};

//Conversion function
void caasBayer2BGR(caasImage* in, caasImage* out);

//Detect function
void caasCLR4TxInspect(const caasImage* image, caasInspectResult* result);

