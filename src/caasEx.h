#pragma once

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

//data structure to hold CLR4Tx inspection result of an image
//NOTE: ALL OPENCV EXCEPTIONS/ASSERTIONA/ERRORS are suppressed. 
//CHECK targetRightEdge, targetLeftEdge, isolatorRightEdge. If at least one of them is -1, something is wrong.
struct caasCLR4TxOutput
{
	int targetRightEdge;
	int targetLeftEdge;

	int isolatorRightEdge;

	int distanceInPixels;
	double distanceInMicrons;

	float isolatorAngle; //should be in -45 ~ 45 degrees

	int isolatorMiddleY; //The Y coordinate of the middle of isolator right edge.
	int baseMiddleY;	//The Y coordinate of the middle of the 2 base circles. Assuming 2 base circles are algined perfectly in vertical.
	double distanceBetweenLinesinMicrons;	//The distance between two middle lines, defined above.

	double processingTime;
};

//Inspection function for CLR4Tx
//IMPORTANT NOTE: 
//The algorithm assumes:
//1. The target component is nearly in perfect vertical orientation.
//2. The width of target component is fixed around 460 microns
//3. Within the range of target component, the top and bottom part is considerable darker than the target component.
//4. The width and height of isolator is fixed round 530 and 400 microns
//5. In the area between the left edge of target component and the left edge of isolator is uniform background, except for the isolator area itself.
void caasCLR4TxInspect(const caasInput* input, caasCLR4TxOutput* output);

//data structure to hold PSM4Tx inspection result of an image
//NOTE: ALL OPENCV EXCEPTIONS/ASSERTIONA/ERRORS are suppressed. 
//CHECK aperture2ndMidY, arrayblock1stMidY, arrayblockRightEdge, isolatorLeftEdge. If at least one of them is -1, something is wrong.
struct caasPSM4TxOutput
{
	int arrayblockRightEdge;
	int isolatorLeftEdge;
	//int apertureMiddleY; //The Y coordinate of the middle of 6 aperatures.
	//int arrayblockMiddleY; //The Y coordinate of the middle of arrayblock.
	int aperture2ndMidY; //The Y coordinate of the middle of the 2nd aperature.
	int arrayblock1stMidY; //The Y coordinate of the middle of 1st arrayblock.

	int horDistanceInPixels;		//The distance between right edge of arrayblock to the left edge of isolator
	double horDistanceInMicrons;

	int verDistanceInPixels;		//The distance between the mid of 1st arrayblock and the mid of 2nd aperture
	double verDistanceInMicrons;

	float angle; //in range of -45 ~ 45 degrees. NOTE: isolator edges are almost invisible and therefore can't detect its angle well.
					//Now, this angel is purely the angle of arrayblock

	double processingTime;
};

//Inspection function for PSM4Tx
void caasPSM4TxInspect(const caasInput* input, caasPSM4TxOutput* output);