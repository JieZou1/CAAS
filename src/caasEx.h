#pragma once

enum PlatformType { CLR4Tx, PSM4Tx };
enum ImageType { BGR, BayerBGGR12, BayerBGGR8};

//data structure to hold the input image
struct caasInput
{
	//Image Data Inputs
	unsigned char* imgData; 
	int imgWidth;
	int imgHeight;
	ImageType imgType;

	//Need to tell the algorithm the accurate pixels per micron, which can not be very different from the case when the algorithm is developed (0.75 pixel per micron)
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

	float isolatorAngle; //should be in -45 ~ 45 degrees

	double processingTime;
};

//Inspection function for CLR4Tx
//IMPORTANT NOTE: 
//For CLR4Tx, the algorithm assumes:
//1. The target component is nearly in perfect vertical orientation.
//2. The width of target component is fixed around 460 microns
//3. Within the range of target component, the top and bottom part is considerable darker than the target component.
//4. The width and height of isolator is fixed round 530 and 400 microns
//5. In the area between the left edge of target component and the left edge of isolator is uniform background, except for the isolator area itself.
//For PSM4Tx, the algorithm assumes:
void caasInspect(const PlatformType platform, const caasInput* input, caasOutput* output);

