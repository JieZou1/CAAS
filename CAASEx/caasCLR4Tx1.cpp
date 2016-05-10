#include "CAASCLR4Tx1.h"

caasCLR4Tx1::caasCLR4Tx1(const caasInput* input) : caasCLR4TxBase(input) {}

/**
Step 1: Find the right edge of target.
We assume the right side of the target is a large black metal. So, it should be darkest region of the image.
We use Otsu method the binarize the image, and then find the right-most largest gradient.
*/
void caasCLR4Tx1::FindTargetRightEdge()
{
	int scale = 10; //We reduce the original image resolution
	int widthSmall = imageGray.cols / scale, heightSmall = imageGray.rows / scale;
	resize(imageGray, imageOneTenth, Size(widthSmall, heightSmall));
#if _DEBUG
	imwrite("1.1.OneTenth.jpg", imageOneTenth);
#endif

	//Otsu binarization
	threshold(imageOneTenth, imageOneTenthOtsu, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
#if _DEBUG
	imwrite("1.2.OneTenthOtsu.jpg", imageOneTenthOtsu);
#endif

	//vertical projection profile
	Mat verProjection(1, imageOneTenthOtsu.cols, CV_32FC1);
	reduce(imageOneTenthOtsu, verProjection, 0, CV_REDUCE_SUM, CV_32FC1); //Vertical projection generate Horizontal profile
	
	float minValue, maxValue; int minIndex, maxIndex; float values[1000]; float gradients[1000];
	ProjectionProfileAnalysis(verProjection, minValue, minIndex, maxValue, maxIndex, values);
	
	//Gradient(widthSmall, values, gradients);
	////Find the largest gradient, that is the position of right edge of target
	////From left to right, this is a negative gradient, i.e., from high gray value to low gray value
	//float minGrad = gradients[widthSmall - 1]; int minGradIndex = widthSmall - 1;
	//for (int i = widthSmall - 2; i > widthSmall / 2; i--)
	//{
	//	if (gradients[i] < minGrad) { minGrad = gradients[i]; minGradIndex = i; }
	//}
	//targetRightEdge = minGradIndex * scale;
	//return;

	//Threshod with maxValue/2, basically assuming targe must be highter than maxValue/2 and matel part must be smaller than maxValue/2.
	for (int i = 0; i < imageOneTenth.cols; i++)		values[i] = values[i] > maxValue / 3 ? 1.0f : 0.0f;

	//Find a run of 1's, which is at least targetWidth/2 long, from the right; if not found, the longest run is used.
	values[imageOneTenth.cols - 1] = 0; //Set the last one as 0, it doesn't matter much, but convenient for finding runs
	int start = -1, end = -1, run; int max_run = 0, max_start = -1, max_end = -1;
	for (int i = imageOneTenth.cols - 2; i >= 0; i--)
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
	targetRightEdge = max_end * scale;
}

/**
Step 2: Find the left edge of the target
Currently we use around 0.75 pixel per micron. So, 10 microns correspond to 7.5 pixels.
We reduce width and height to 1/4, which makes 10 microns correspond to 1.875 pixels, 50 microns --> 9.375 pixels, 
which should be large enough to have a dark strip between isolator right edge and target left edge. This dark strip is for detecting left edge of target.

We assume the target area should have high gradients, and the drak strip should have almost no gradients. 
So, we inspect the center 1/2 of the height of the image, and look for sudden gradient changes.
*/
void caasCLR4Tx1::FindTargetLeftEdge()
{
	int scale = 4; //We reduce the original image resolution

	resize(this->imageGray, imageOneFourth, Size(this->imageGray.cols / scale, this->imageGray.rows / scale));
	//Rect roi = Rect(0, 0, targetRightEdge / scale, imageGrayQuarter.rows);
	Rect roi = Rect(0, imageOneFourth.rows / 4, targetRightEdge / scale, imageOneFourth.rows / 2);
	imageOneFourthMiddleHalf = imageOneFourth(roi);
#if _DEBUG
	imwrite("2.1.OneFourthMiddleHalf.jpg", imageOneFourthMiddleHalf);
#endif

	//sharpen the image
	//Unsharping masking: Use a Gaussian smoothing filter and subtract the smoothed version from the original image (in a weighted way so the values of a constant area remain constant). 
	Mat imageBlurred, imageGraySharpened;	double GAUSSIAN_RADIUS = 4.0;
	GaussianBlur(imageOneFourthMiddleHalf, imageBlurred, Size(0, 0), GAUSSIAN_RADIUS);
	addWeighted(imageOneFourthMiddleHalf, 1.5, imageBlurred, -0.5, 0, imageGraySharpened);
#if _DEBUG
	//imageGraySharpened = imageGrayQuarter;
	imwrite("2.2.OneFourthMiddleHalfSharpened.jpg", imageGraySharpened);
#endif

	//Histogram Equalization
	equalizeHist(imageGraySharpened, imageGraySharpened);
#if _DEBUG
	imwrite("2.3.OneFourthMiddleHalfEqualized.jpg", imageGraySharpened);
#endif

	//Canny Edge Detection
	int median = Median(imageGraySharpened);
	Canny(imageGraySharpened, imageOneFourthMiddleHalfCanny, 0.66 * median, 1.33 * median);
#if _DEBUG
	imwrite("2.4.OneFourthMiddleHalfCanny.jpg", imageOneFourthMiddleHalfCanny);
#endif

	//vertical projection profile
	Mat verProjection(1, imageOneFourthMiddleHalfCanny.cols, CV_32FC1);
	reduce(imageOneFourthMiddleHalfCanny, verProjection, 0, CV_REDUCE_SUM, CV_32FC1); //Vertical projection generate Horizontal profile

	float minValue = -1.0, maxValue = -1.0; int minIndex, maxIndex; float values[4000]; float gradients[4000];
	ProjectionProfileAnalysis(verProjection, minValue, minIndex, maxValue, maxIndex, values);
	
	//Gradient(imageOneFourthMiddleHalfCanny.cols, values, gradients);
	////Find the largest gradient, that is the position of right edge of target
	////From left to right, this is a positive gradient, i.e., from low gradent value to high gradent value
	//float maxGrad = gradients[imageOneFourthMiddleHalfCanny.cols - 1]; int maxGradIndex = imageOneFourthMiddleHalfCanny.cols - 1;
	//for (int i = 1; i < 2 * (targetWidth / scale); i++) //We search in only 2 times of the expected target width
	//{
	//	int index = imageOneFourthMiddleHalfCanny.cols - 1 - i;
	//	if (gradients[index] > maxGrad) { maxGrad = gradients[index]; maxGradIndex = index; }
	//}
	//targetLeftEdge = maxGradIndex * scale;
	//return;

	//Find the average profile vaule in 3/4 of the target region
	float average = 0; int w = 3 * (targetWidth / 4) / 4;
	for (int i = 0; i < w; i++)		average += values[imageOneFourthMiddleHalfCanny.cols - 1 - i];
	average /= w;
	
	//Find the left edge of target
	//Search to left for a targtWidth, and find adrupt change
	float max_diff = -1; int max_pos = -1;
	for (int i = 0; i < targetWidth / scale; i++)
	{
		float value0 = values[imageOneFourthMiddleHalfCanny.cols - 1 - w - i];
		float value1 = values[imageOneFourthMiddleHalfCanny.cols - w - i];
		if (value0 > average / 2) continue;	//We are expecting the left of the edge has a lot less edges than arerage target area.
		//if (value1 < average) continue;	//We are expecting the edge has a lot less edges than the average target area.
		float diff = value1 - value0;
		if (diff > max_diff)
		{
			max_diff = diff; max_pos = imageOneFourthMiddleHalfCanny.cols - 1 - w - i;
		}
	}
	targetLeftEdge = max_pos * scale;
}

/**
Step 3: Find the top and bottom edge of the target.
We assume that the regions above and below the target are darker than the target.
*/
void caasCLR4Tx1::FindTargetTopBottomEdges()
{
	int scale = 4;

	Rect roi = Rect(targetLeftEdge / scale, 0, (targetRightEdge - targetLeftEdge) / scale, this->imageOneFourth.rows);
	imageOneFourthTarget = this->imageOneFourth(roi);
#if _DEBUG
	imwrite("3.1.OneFourthTarget.jpg", imageOneFourthTarget);
#endif

	Mat imageOtsu; threshold(imageOneFourthTarget, imageOtsu, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
#if _DEBUG
	imwrite("3.2.OneFourthTargetOtsu.jpg", imageOtsu);
#endif
	//horizontal projection profile
	Mat horProjection(imageOtsu.rows, 1, CV_32FC1);
	reduce(imageOtsu, horProjection, 1, CV_REDUCE_SUM, CV_32FC1); //horizontal projection generate vertical profile

	float minValue = -1.0, maxValue = -1.0; int minIndex, maxIndex; float values[3000]; float gradients[3000];
	ProjectionProfileAnalysis(horProjection, minValue, minIndex, maxValue, maxIndex, values);
	Gradient(horProjection.rows, values, gradients);

	//Find the top edge of the target
	//From top to bottom, this is a positive gradient, i.e., from low gray value to high gray value
	float maxGrad = gradients[0]; int maxGradIndex = 0;
	for (int i = 1; i < horProjection.rows / 3; i++) //We search in only top 1/3
	{
		if (gradients[i] > maxGrad) { maxGrad = gradients[i]; maxGradIndex = i; }
	}
	targetTopEdge = maxGradIndex * scale;

	//Find the bottom edge of the target
	//From top to bottom, this is a negative gradient, i.e., from high gray value to low gray value
	float minGrad = gradients[horProjection.rows - 1]; int minGradIndex = horProjection.rows - 1;
	for (int i = horProjection.rows - 2; i >= 2 * horProjection.rows / 3; i--) //We search in only bottom 1/3
	{
		if (gradients[i] < minGrad) { minGrad = gradients[i]; minGradIndex = i; }
	}
	targetBottomEdge = minGradIndex * scale;

#if _DEBUG
	roi = Rect(targetLeftEdge, targetTopEdge, targetRightEdge - targetLeftEdge, targetBottomEdge - targetTopEdge);
	imageTarget = imageGray(roi);
	imwrite("3.3.Target.jpg", imageTarget);
#endif
	return;

	////Find the top edge of the target
	//for (int i = 0; i < horProjection.rows; i++)
	//{
	//	if (values[i] > maxValue / 2)
	//	{
	//		targetTopEdge = i*scale; break;
	//	}
	//}
	////Find the bottom edge of the target
	//for (int i = horProjection.rows - 1; i >= 0; i--)
	//{
	//	if (values[i] > maxValue / 2)
	//	{
	//		targetBottomEdge = i*scale; break;
	//	}
	//}
	//return;
}

/**
Step 4: Find Isolator and its orientation
Currently we use around 0.75 pixel per micron. So, 10 microns correspond to 7.5 pixels.
We reduce width and height to 1/4, which makes 10 microns correspond to 1.875 pixels, 50 microns --> 9.375 pixels,

We assume the isolator has high texture, and therefore high gradients, and the regions between isolator and target has less gradients.
*/
void caasCLR4Tx1::FindIsolator()
{
	int high = targetBottomEdge - targetTopEdge, top = targetTopEdge + high / 4;
	RoiTargetLeftMiddleHalf = Rect(0, top, targetLeftEdge, high / 2); //We assume the isolator will be in the middle part of the target.
	imageTargetLeftMiddleHalf = imageGray(RoiTargetLeftMiddleHalf);
#if _DEBUG
	imwrite("4.1.TargetLeftMiddleHalf.jpg", imageTargetLeftMiddleHalf);
#endif

	int scale = 4; //We reduce the original image resolution

	resize(imageTargetLeftMiddleHalf, imageOneFourthTargetLeftMiddleHalf, Size(imageTargetLeftMiddleHalf.cols / scale, imageTargetLeftMiddleHalf.rows / scale));
#if _DEBUG
	imwrite("4.2.OneFourthTargetLeftMiddleHalf.jpg", imageOneFourthTargetLeftMiddleHalf);
#endif

	//sharpen the image
	//Unsharping masking: Use a Gaussian smoothing filter and subtract the smoothed version from the original image (in a weighted way so the values of a constant area remain constant). 
	Mat imageBlurred, imageGraySharpened;	double GAUSSIAN_RADIUS = 4.0;
	GaussianBlur(imageOneFourthTargetLeftMiddleHalf, imageBlurred, Size(0, 0), GAUSSIAN_RADIUS);
	addWeighted(imageOneFourthTargetLeftMiddleHalf, 1.5, imageBlurred, -0.5, 0, imageGraySharpened);
#if _DEBUG
	//imageGraySharpened = imageGrayQuarter;
	imwrite("4.3.OneFourthTargetLeftMiddleHalfSharpened.jpg", imageGraySharpened);
#endif

	//Histogram Equalization
	equalizeHist(imageGraySharpened, imageGraySharpened);
#if _DEBUG
	imwrite("4.4.OneFourthTargetLeftMiddleHalfEqualized.jpg", imageGraySharpened);
#endif

	//Canny Edge Detection
	int median = Median(imageGraySharpened);
	Canny(imageGraySharpened, imageOneFourthTargetLeftMiddleHalfCanny, 0.66 * median, 1.33 * median);
#if _DEBUG
	imwrite("4.5.OneFourthTargetLeftMiddleHalfCanny.jpg", imageOneFourthTargetLeftMiddleHalfCanny);
#endif

	{	//Find the isolator right edge
		//vertical projection profile
		Mat verProjection(1, imageOneFourthTargetLeftMiddleHalfCanny.cols, CV_32FC1);
		reduce(imageOneFourthTargetLeftMiddleHalfCanny, verProjection, 0, CV_REDUCE_SUM, CV_32FC1); //Vertical projection generate Horizontal profile
		float minValue = -1.0, maxValue = -1.0; int minIndex, maxIndex; float values[4000];
		ProjectionProfileAnalysis(verProjection, minValue, minIndex, maxValue, maxIndex, values);

		//Set the start and end 3 to be zero for convenience
		vector<int> starts, ends; int start, end;
		for (int i = 0; i < 3; i++)		{			values[verProjection.cols - 1 - i] = 0;			values[i] = 0;		}

		//Find all non-zero runs
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
	}
	{	//Find isolator top and bottom edges
		//We just look at the half width of the isolator, trying to find its top and bottom edges
		isolatorLeftEdge = isolatorRightEdge - isolatorWidth / 2;
		Rect roi = Rect(isolatorLeftEdge / scale, 0, (isolatorRightEdge - isolatorLeftEdge) / scale, imageOneFourthTargetLeftMiddleHalfCanny.rows);
		imageIsolatorRoiCanny = imageOneFourthTargetLeftMiddleHalfCanny(roi);
#if _DEBUG
		imwrite("4.6.IsolatorRoiCanny.jpg", imageIsolatorRoiCanny);
#endif

		//horizontal projection profile
		Mat horProjection(imageIsolatorRoiCanny.rows, 1, CV_32FC1);
		reduce(imageIsolatorRoiCanny, horProjection, 1, CV_REDUCE_SUM, CV_32FC1); //horizontal projection generate vertical profile
		float minValue = -1.0, maxValue = -1.0; int minIndex, maxIndex; float values[4000];
		ProjectionProfileAnalysis(horProjection, minValue, minIndex, maxValue, maxIndex, values);

		//Find all 0's and remove 0's which are smaller than a threshold
		float prev, curr; vector<int> starts, ends;
		for (int i = 0; i <= imageIsolatorRoiCanny.rows; i++)
		{
			prev = i == 0 ? prev = 1 : values[i - 1];
			curr = i == imageIsolatorRoiCanny.rows ? 1 : values[i];

			if (prev > 0.5 && curr < 0.5) starts.push_back(i);
			if (prev < 0.5 && curr > 0.5) ends.push_back(i);
		}
		if (starts.size() == 0)
		{
			isolatorTopEdge = roi.y * scale; 
			isolatorBottomEdge = (roi.y + roi.height) * scale;
		}
		else
		{
			vector<int> zero_starts, zero_ends;
			for (int i = 0; i < starts.size(); i++)
			{
				int length = ends[i] - starts[i];
				if (length > 5) { zero_starts.push_back(starts[i]); zero_ends.push_back(ends[i]); }
			}
			//Pick one 1's, which is closest to the expected isolator width
			vector<int> one_starts, one_ends;
			if (zero_starts[0] != 0) { one_starts.push_back(0); one_ends.push_back(zero_starts[0]); }
			for (int i = 0; i < zero_starts.size() - 1; i++)
			{
				one_starts.push_back(zero_ends[i]);
				one_ends.push_back(zero_starts[i + 1]);
			}
			if (zero_ends[zero_ends.size() - 1] != imageIsolatorRoiCanny.rows) { one_starts.push_back(zero_ends[zero_ends.size() - 1]); one_ends.push_back(imageIsolatorRoiCanny.cols); }

			int min_index, min_diff = 4000;
			for (int i = 0; i < one_starts.size(); i++)
			{
				int length = one_ends[i] - one_starts[i];
				int diff = std::abs(length - this->isolatorWidth / scale);
				if (diff < min_diff) { min_diff = diff; min_index = i; }
			}

			isolatorTopEdge = this->RoiTargetLeftMiddleHalf.y + (roi.y + one_starts[min_index]) * scale;
			isolatorBottomEdge = this->RoiTargetLeftMiddleHalf.y + (roi.y + one_ends[min_index]) * scale;
		}
#if _DEBUG
		roi = Rect(isolatorLeftEdge, isolatorTopEdge, isolatorRightEdge - isolatorLeftEdge, isolatorBottomEdge - isolatorTopEdge);
		imageIsolator = imageGray(roi);
		imwrite("4.7.Isolator.jpg", imageIsolator);
#endif
	}
}

void caasCLR4Tx1::FindIsolatorAngle()
{
	//We cut 4/5 width of isolator out
	int height = 3 * (isolatorBottomEdge - isolatorTopEdge) / 2; int middle = (isolatorBottomEdge + isolatorTopEdge) / 2;
	Rect rect = Rect(isolatorRightEdge - 4 * isolatorWidth / 5, middle - height / 2, 4 * isolatorWidth / 5, height);
	Mat imageIsolator = imageGray(rect);
	int scale = 4;
	resize(imageIsolator, imageIsolator, Size(imageIsolator.cols / scale, imageIsolator.rows / scale));
#if _DEBUG
	imwrite("Isolator.jpg", imageIsolator);
#endif

	//sharpen the image
	//Unsharping masking: Use a Gaussian smoothing filter and subtract the smoothed version from the original image (in a weighted way so the values of a constant area remain constant). 
	Mat imageBlurred, imageGraySharpened;	double GAUSSIAN_RADIUS = 4.0;
	GaussianBlur(imageIsolator, imageBlurred, Size(0, 0), GAUSSIAN_RADIUS);
	addWeighted(imageIsolator, 1.5, imageBlurred, -0.5, 0, imageGraySharpened);
#if _DEBUG
	//imageGraySharpened = imageGrayQuarter;
	imwrite("IsolatorSharpened.jpg", imageGraySharpened);
#endif

	imageGraySharpened = imageIsolator;

	//Histogram Equalization
	equalizeHist(imageGraySharpened, imageGraySharpened);
#if _DEBUG
	imwrite("IsolatorEqualized.jpg", imageGraySharpened);
#endif

	//Otsu binarization
	Mat imageOtsu; threshold(imageGraySharpened, imageOtsu, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
#if _DEBUG
	imwrite("IsolatorOtsu.jpg", imageOtsu);
#endif

	//Canny Edge Detection
	int median = Median(imageGraySharpened);
	Mat imageCanny;  Canny(imageGraySharpened, imageCanny, 0.66 * median, 1.33 * median);
#if _DEBUG
	imwrite("IsolatorCanny.jpg", imageCanny);
#endif

}

void caasCLR4Tx1::RefineIsolator()
{
	//int width = isolatorRightEdge - isolatorLeftEdge;
	//int height = isolatorBottomEdge - isolatorTopEdge;
	//Expand horizontally 1 time to the right; expand vertically 1/2

	int left = isolatorLeftEdge;
	int right = left + 2 * isolatorWidth; if (right > targetLeftEdge - 20) right = targetLeftEdge - 20;
	int top = isolatorTopEdge - isolatorHeight / 4;
	int bottom = top + isolatorHeight + isolatorHeight / 2;
	Rect roi = Rect(left, top, right - left, bottom - top);
	imageIsolatorRoi = imageGray(roi);
#if _DEBUG
	imwrite("5.1.IsolatorRoi.jpg", imageIsolatorRoi);
#endif

	int scale = 4;
	resize(imageIsolatorRoi, imageIsolatorRoi, Size(imageIsolatorRoi.cols / scale, imageIsolatorRoi.rows / scale));

	Mat imageBlurred, imageGraySharpened;	double GAUSSIAN_RADIUS = 4.0;
	GaussianBlur(imageIsolatorRoi, imageBlurred, Size(0, 0), GAUSSIAN_RADIUS);
	addWeighted(imageIsolatorRoi, 2.5, imageBlurred, -1.5, 0, imageGraySharpened);
#if _DEBUG
	imwrite("5.2.IsolatorRoiSharpened.jpg", imageGraySharpened);
#endif

	int median = Median(imageGraySharpened);
	Mat imageCanny; Canny(imageGraySharpened, imageCanny, 0.66 * median, 1.33 * median);
#if _DEBUG
	imwrite("5.3.IsolatorRoiCanny.jpg", imageCanny);
#endif

	Mat Points;	findNonZero(imageCanny, Points);
	Rect Min_Rect = boundingRect(Points);
	isolatorRightEdge = roi.x + (Min_Rect.x + Min_Rect.width) * scale;
	RotatedRect rect = minAreaRect(Points);
	isolatorAngle = rect.angle;
}

void caasCLR4Tx1::Inspect()
{
#if _DEBUG
	imwrite("0.1.gray.jpg", imageGray);
#endif

	FindTargetRightEdge();
	FindTargetLeftEdge();
	FindTargetTopBottomEdges();
	FindIsolator();
	//FindIsolatorAngle();
	RefineIsolator();
	return;
}

