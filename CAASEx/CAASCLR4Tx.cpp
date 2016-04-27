#include "CAASCLR4Tx.h"

caasCLR4Tx::caasCLR4Tx(const caasInput* input) : caasBase(input)
{
	targetWidth = (int)(input->pixelsPerMicron * TARGET_WIDTH_MICRON);
}

int caasCLR4Tx::FindTargetMetalEdge()
{
	int scale = 10; //We reduce the original image resolution

	Mat imageSmall;
	resize(imageGray, imageSmall, Size(imageGray.cols / scale, imageGray.rows / scale));
	imwrite("small.jpg", imageSmall);

	//Otsu binarization
	threshold(imageSmall, imageOtsu, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
	imwrite("otsu.jpg", imageOtsu);

	//vertical projection profile
	Mat verProjection(1, imageOtsu.cols, CV_32FC1);
	reduce(imageOtsu, verProjection, 0, CV_REDUCE_SUM, CV_32FC1); //Vertical projection generate Horizontal profile

	float minValue = -1.0, maxValue = -1.0; int minIndex, maxIndex; float* values = new float[imageSmall.cols];
	{
		MatIterator_<float> it, end; float value; int i;
		for (i = 0, it = verProjection.begin<float>(), end = verProjection.end<float>(); it != end; ++it, ++i)
		{
			value = *it / 255; values[i] = value;
			if (minValue < 0) { minValue = maxValue = value; minIndex = maxIndex = 0; continue; } //first time.
			if (value < minValue) { minValue = value; minIndex = i; }
			if (value > maxValue) { maxValue = value; maxIndex = i; }
		}
	}

	//Threshod with maxValue/2, basically assuming targe must be highter than maxValue/2 and matel part must be smaller than maxValue/2.
	for (int i = 0; i < imageSmall.cols; i++)		values[i] = values[i] > maxValue / 2 ? 1.0f : 0.0f;

	//Find a run of 1's, which is at least targetWidth/2 long, from the right; if not found, the longest run is used.
	values[imageSmall.cols - 1] = 0; //Set the last one as 0, it doesn't matter much, but convenient for finding runs
	int start = -1, end = -1, run; int max_run = 0, max_start = -1, max_end = -1;
	for (int i = imageSmall.cols - 2; i >= 0; i--)
	{
		if (values[i + 1] > 0.5 && values[i] > 0.5) continue;
		if (values[i + 1] < 0.5 && values[i] < 0.5) continue;
		if (values[i + 1] < 0.5 && values[i] > 0.5)	{	end = i; continue;		}
		if (values[i + 1] > 0.5 && values[i] < 0.5)	
		{ 
			start = i; run = end - start;
			if (run > targetWidth / 2.0)
			{
				max_start = start; max_end = end; 	max_run = run; 
				break;
			}
			else
			{
				if (run > max_run)
				{
					max_start = start; max_end = end; 	max_run = run;
				}
			}
		}
	}

	delete[] values;

	return max_end * scale;
}

void caasCLR4Tx::FindTargetEdge()
{
	int scale = 4; //We reduce the original image resolution

	resize(imageGray, imageGrayQuarter, Size(imageGray.cols / scale, imageGray.rows / scale));

	Rect roi = Rect(0, 0, metalPosition / scale, imageGrayQuarter.rows);
	imageGrayQuarter = imageGrayQuarter(roi);
	imwrite("quarter.jpg", imageGrayQuarter);

	//sharpen the image
	//Unsharping masking: Use a Gaussian smoothing filter and subtract the smoothed version from the original image (in a weighted way so the values of a constant area remain constant). 
	Mat imageBlurred;	double GAUSSIAN_RADIUS = 4.0;
	GaussianBlur(imageGrayQuarter, imageBlurred, Size(0, 0), GAUSSIAN_RADIUS);
	addWeighted(imageGrayQuarter, 1.5, imageBlurred, -0.5, 0, imageGraySharpened);
	//imageGraySharpened = imageGrayQuarter;
	imwrite("Sharpened.jpg", imageGraySharpened);

	//Histogram Equalization
	equalizeHist(imageGraySharpened, imageGraySharpened);
	imwrite("Equalized.jpg", imageGrayQuarter);

	//Canny Edge Detection
	int median = Median(imageGraySharpened);
	Canny(imageGraySharpened, imageCanny, 0.66 * median, 1.33 * median);
	imwrite("Canny.jpg", imageCanny);

	//vertical projection profile
	Mat verProjection(1, imageCanny.cols, CV_32FC1);
	reduce(imageCanny, verProjection, 0, CV_REDUCE_SUM, CV_32FC1); //Vertical projection generate Horizontal profile

	float minValue = -1.0, maxValue = -1.0; int minIndex, maxIndex; float values[4000];
	{
		MatIterator_<float> it, end; float value; int i;
		for (i = 0, it = verProjection.begin<float>(), end = verProjection.end<float>(); it != end; ++it, ++i)
		{
			value = *it / 255; values[i] = value;
			if (minValue < 0) { minValue = maxValue = value; minIndex = maxIndex = 0; continue; } //first time.
			if (value < minValue) { minValue = value; minIndex = i; }
			if (value > maxValue) { maxValue = value; maxIndex = i; }
		}
	}
	//minMaxIdx(verProjection, &minValue, &maxValue, &minIndex, &maxIndex, noArray());

}

void caasCLR4Tx::Inspect()
{
	//Find the edge of targe and black matel.
	metalPosition = FindTargetMetalEdge();

	FindTargetEdge();
	return;


	////
	//lsd = createLineSegmentDetector();
	////lsd = createLineSegmentDetector(LSD_REFINE_ADV);
	//lsd->detect(imageCanny, this->lsdLines);
	//Mat black(imageGray.rows / 4, imageGray.cols / 4, CV_8UC3, Scalar(0, 0, 0));
	//lsd->drawSegments(black, lsdLines);
	//imwrite("CannyLine.jpg", black);

	//int scale = 1;
	//int delta = 0;
	//int ddepth = CV_16S;
	//Sobel(imageGraySharpened, imageSobelX, ddepth, 1, 0, 3, scale, delta, BORDER_DEFAULT);
	//convertScaleAbs(imageSobelX, imageSobelX);
	//imwrite("SobelX.jpg", imageSobelX);

	//Sobel(imageGraySharpened, imageSobelY, ddepth, 0, 1, 3, scale, delta, BORDER_DEFAULT);
	//convertScaleAbs(imageSobelY, imageSobelY);
	//imwrite("SobelY.jpg", imageSobelY);

	//addWeighted(imageSobelX, 0.5, imageSobelY, 0.5, 0, imageSobel);
	//imwrite("Sobel.jpg", imageSobel);

	//lsd->detect(this->imageSobelX, this->lsdLines);
	//lsd->drawSegments(imageSobelX, lsdLines);
	//imwrite("SobelLine.jpg", imageSobelX);



}

void caasCLR4Tx::GetResult(caasOutput* result)
{
	//TODO filling result struct.

	//After the result is filled, we calculate processing time.
	endT = std::clock();	result->processingTime = (endT - startT) / (double)CLOCKS_PER_SEC;

}
