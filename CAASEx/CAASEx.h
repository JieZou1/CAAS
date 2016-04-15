#pragma once

//data structure to hold the input image
struct caasInputImage
{
	unsigned char* data; //Has to be in BGR 8 bit format
	int width;
	int height;
};

//data structure to hold inspection result of an image
struct caasInspectResult
{
	double processingTime;
};

//Detect function
void caasCLR4TxInspect(const caasInputImage* image, caasInspectResult* result);

