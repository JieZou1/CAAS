#include "CAASCLR4Tx.h"

caasCLR4Tx::caasCLR4Tx(const caasInput* input) : caasBase(input)
{
	targetWidth = (int)(input->pixelsPerMicron * TARGET_WIDTH_MICRON);

	isolatorWidth = (int)(1.16 * targetWidth);
	isolatorHeight = (int)(0.87 * targetWidth);

	targetRightEdge = targetLeftEdge = isolatorRightEdge = -1;
}

void caasCLR4Tx::FindTargetRightEdge()
{
	int scale = 10; //We reduce the original image resolution

	Mat imageSmall;	resize(imageGray, imageSmall, Size(imageGray.cols / scale, imageGray.rows / scale));
#if _DEBUG
	imwrite("small.jpg", imageSmall);
#endif

	//Otsu binarization
	Mat imageOtsu; threshold(imageSmall, imageOtsu, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
#if _DEBUG
	imwrite("otsu.jpg", imageOtsu);
#endif

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

	targetRightEdge = max_end * scale;
}

void caasCLR4Tx::FindTargetLeftEdge()
{
	int scale = 4; //We reduce the original image resolution

	resize(imageGray, imageGrayQuarter, Size(imageGray.cols / scale, imageGray.rows / scale));

	Rect roi = Rect(0, 0, targetRightEdge / scale, imageGrayQuarter.rows);
	imageGrayQuarter = imageGrayQuarter(roi);
#if _DEBUG
	imwrite("quarter.jpg", imageGrayQuarter);
#endif

	//sharpen the image
	//Unsharping masking: Use a Gaussian smoothing filter and subtract the smoothed version from the original image (in a weighted way so the values of a constant area remain constant). 
	Mat imageBlurred, imageGraySharpened;	double GAUSSIAN_RADIUS = 4.0;
	GaussianBlur(imageGrayQuarter, imageBlurred, Size(0, 0), GAUSSIAN_RADIUS);
	addWeighted(imageGrayQuarter, 1.5, imageBlurred, -0.5, 0, imageGraySharpened);
#if _DEBUG
	//imageGraySharpened = imageGrayQuarter;
	imwrite("Sharpened.jpg", imageGraySharpened);
#endif

	//Histogram Equalization
	equalizeHist(imageGraySharpened, imageGraySharpened);
#if _DEBUG
	imwrite("Equalized.jpg", imageGraySharpened);
#endif

	//Canny Edge Detection
	int median = Median(imageGraySharpened);
	Mat imageCanny;  Canny(imageGraySharpened, imageCanny, 0.66 * median, 1.33 * median);
#if _DEBUG
	imwrite("Canny.jpg", imageCanny);
#endif

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

	//Find the average profile vaule in 3/4 of the target region
	float average = 0; int w = 3 * (targetWidth / 4) / 4;
	for (int i = 0; i < w; i++)		average += values[imageCanny.cols - 1 - i];
	average /= w;
	
	//Find the left edge of target
	//Search to left for a targtWidth, and find adrupt change
	float max_diff = -1; int max_pos = -1;
	for (int i = 0; i < targetWidth; i++)
	{
		float value0 = values[imageCanny.cols - 1 - w - i];
		float value1 = values[imageCanny.cols - w - i];
		if (value0 > average / 2) continue;	//We are expecting the left of the edge has a lot less edges than arerage target area.
		if (value1 < average) continue;	//We are expecting the edge has a lot less edges than the average target area.
		float diff = value1 - value0;
		if (diff > max_diff)
		{
			max_diff = diff; max_pos = imageCanny.cols - 1 - w - i;
		}
	}
	targetLeftEdge = max_pos * scale;
}

void caasCLR4Tx::FindTargetTopBottomEdges()
{
	int scale = 4;
	Rect roi = Rect(targetLeftEdge / scale, 0, (targetRightEdge - targetLeftEdge) / scale, imageGrayQuarter.rows);
	Mat imageTargetQuarter = imageGrayQuarter(roi);
#if _DEBUG
	imwrite("Target.jpg", imageTargetQuarter);
#endif

	Mat imageOtsu; threshold(imageTargetQuarter, imageOtsu, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
#if _DEBUG
	imwrite("TargetOtsu.jpg", imageOtsu);
#endif
	//horizontal projection profile
	Mat horProjection(1, imageOtsu.cols, CV_32FC1);
	reduce(imageOtsu, horProjection, 1, CV_REDUCE_SUM, CV_32FC1); //horizontal projection generate vertical profile

	float minValue = -1.0, maxValue = -1.0; int minIndex, maxIndex; float values[3000];
	{
		MatIterator_<float> it, end; float value; int i;
		for (i = 0, it = horProjection.begin<float>(), end = horProjection.end<float>(); it != end; ++it, ++i)
		{
			value = *it / 255; values[i] = value;
			if (minValue < 0) { minValue = maxValue = value; minIndex = maxIndex = 0; continue; } //first time.
			if (value < minValue) { minValue = value; minIndex = i; }
			if (value > maxValue) { maxValue = value; maxIndex = i; }
		}
	}

	//Find the top edge of the target
	for (int i = 0; i < horProjection.rows; i++)
	{
		if (values[i] > maxValue / 2)
		{
			targetTopEdge = i*scale; break;
		}
	}
	//Find the bottom edge of the target
	for (int i = horProjection.rows - 1; i >= 0; i--)
	{
		if (values[i] > maxValue / 2)
		{
			targetBottomEdge = i*scale; break;
		}
	}
	return;
}

void caasCLR4Tx::FindIsolator()
{
	int high = targetBottomEdge - targetTopEdge;
	int top = targetTopEdge + high / 4;
	Rect roi = Rect(0, top, targetLeftEdge, high / 2); //We assume the isolator will be in the middle part of the target.
	Mat imageIsolatorROI = imageGray(roi);
#if _DEBUG
	imwrite("Isolator.jpg", imageIsolatorROI);
#endif

	//Mat imageOtsu; threshold(imageIsolatorROI, imageOtsu, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
	//imwrite("IsolatorOtsu.jpg", imageOtsu);

	int scale = 4; //We reduce the original image resolution

	Mat imageIsolatorROIQuartor; resize(imageIsolatorROI, imageIsolatorROIQuartor, Size(imageIsolatorROI.cols / scale, imageIsolatorROI.rows / scale));
#if _DEBUG
	imwrite("IsolatorQuarter.jpg", imageIsolatorROIQuartor);
#endif

	//sharpen the image
	//Unsharping masking: Use a Gaussian smoothing filter and subtract the smoothed version from the original image (in a weighted way so the values of a constant area remain constant). 
	Mat imageBlurred, imageGraySharpened;	double GAUSSIAN_RADIUS = 4.0;
	GaussianBlur(imageIsolatorROIQuartor, imageBlurred, Size(0, 0), GAUSSIAN_RADIUS);
	addWeighted(imageIsolatorROIQuartor, 1.5, imageBlurred, -0.5, 0, imageGraySharpened);
#if _DEBUG
	//imageGraySharpened = imageGrayQuarter;
	imwrite("IsolatorSharpened.jpg", imageGraySharpened);
#endif

	//Histogram Equalization
	equalizeHist(imageGraySharpened, imageGraySharpened);
#if _DEBUG
	imwrite("IsolatorEqualized.jpg", imageGraySharpened);
#endif

	//Canny Edge Detection
	int median = Median(imageGraySharpened);
	Mat imageCanny;  Canny(imageGraySharpened, imageCanny, 0.66 * median, 1.33 * median);
#if _DEBUG
	imwrite("IsolatorCanny.jpg", imageCanny);
#endif

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

	//Find all non-zero runs
	vector<int> starts, ends; int start, end;
	for (int i = 0; i < 3; i++)
	{
		values[verProjection.cols - 1 - i] = 0; //Set the end 3 to be zero for convenience
		values[i] = 0; //Set the start 3 to be zero for convenience
	}
	
	for (int i = verProjection.cols - 2; i >= 0; i--)
	{
		if (values[i + 1] < 0.5 && values[i] < 0.5) continue;
		if (values[i + 1] > 0.5 && values[i] > 0.5) continue;
		if (values[i + 1] < 0.5 && values[i] > 0.5) { end = i; continue; }
		if (values[i + 1] > 0.5 && values[i] < 0.5)
		{
			start = i + 1; starts.push_back(start); ends.push_back(end);
		}
	}

	//Check every runs, cut 3/4 of the expected isolator width, and we expect to see many edges (high profile values)
	for (int i = 0; i < starts.size(); i++)
	{
		end = ends[i]; start = end - 3 * (isolatorWidth / scale) / 4;
		float average = 0;
		for (int k = start; k <= end; k++)			average += values[k];
		average /= (float)(end - start + 1);

		if (average < 3) continue; //Has to be larger than a threshold

		isolatorRightEdge = end * scale;
		break;
	}

	//Crop the part of isolator
	roi = Rect(start, 0, end - start, imageCanny.rows);
	imageCanny = imageCanny(roi);
	Mat imageIsolator = imageIsolatorROIQuartor(roi);
#if _DEBUG
	imwrite("Isolator.jpg", imageIsolator);
	imwrite("IsolatorCanny.jpg", imageCanny);
#endif

	//TODO: find the angle

}

void caasCLR4Tx::Inspect()
{
	//Find the edge of targe and black matel.
	FindTargetRightEdge();
	FindTargetLeftEdge();
	FindTargetTopBottomEdges();
	FindIsolator();
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
	//Filling result struct.
	result->targetRightEdge = targetRightEdge;
	result->targetLeftEdge = targetLeftEdge;
	result->isolatorRightEdge = isolatorRightEdge;

	result->distanceInPixels = targetLeftEdge - isolatorRightEdge;
	result->distanceInMicrons = result->distanceInPixels / pixelsPerMicron;

	//After the result is filled, we calculate processing time.
	endT = std::clock();	result->processingTime = (endT - startT) / (double)CLOCKS_PER_SEC;

}
