#pragma once

enum ImageType { BGR, BayerBGGR12};

//data structure to hold the input image
struct caasInput
{
	//Image Data Inputs
	unsigned char* imgData; 
	int imgWidth;
	int imgHeight;
	ImageType imgType;

	//Need to tell the algorithm the accurate pixels per micron, which can not be very different from the case when the algorithm is developed.
	double pixelsPerMicron;

	caasInput(int image_width, int image_height, ImageType image_type, unsigned char* image_data, double pixels_per_micron)
	{
		imgData = image_data;
		imgWidth = image_width;
		imgHeight = image_height;
		imgType = image_type;
		pixelsPerMicron = pixels_per_micron;
	}
};

//data structure to hold inspection result of an image
struct caasOutput
{
	int targetRightEdge;
	int targetLeftEdge;
	int isolatorRightEdge;

	int distanceInPixels;
	double distanceInMicrons;

	double processingTime;
};

//Inspection function
void caasCLR4TxInspect(const caasInput* input, caasOutput* output);

