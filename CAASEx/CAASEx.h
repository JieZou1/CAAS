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

	caasInput(int image_width, int image_height, ImageType image_type, unsigned char* image_data)
	{
		imgData = image_data;
		imgWidth = image_width;
		imgHeight = image_height;
		imgType = image_type;
	}
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

