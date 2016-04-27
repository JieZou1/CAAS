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
//NOTE: ALL OPENCV EXCEPTIONS/ASSERTIONA/ERRORS are suppressed. 
//CHECK targetRightEdge, targetLeftEdge, isolatorRightEdge. If at least one of them is -1, something is wrong.
struct caasOutput
{
	int targetRightEdge;
	int targetLeftEdge;
	int isolatorRightEdge;

	int distanceInPixels;
	double distanceInMicrons;

	double processingTime;
};

//Inspection function for CLR4Tx
//IMPORTANT NOTE: The algorithm has assumed:
//1. On the right side of and connected to the target component, there is a considerable large black matel surface.
//2. The target component is nearly in perfect vertical orientation.
//3. The width of target component is fixed around 460 microns
//4. Within the range of target component, the top and bottom part is considerable darker than the target component.
//5. The width and height of isolator is fixed round 530 and 400 microns
//6. In the area between the left edge of target component and the left edge of isolator is uniform background, except for the isolator area itself.
void caasCLR4TxInspect(const caasInput* input, caasOutput* output);

