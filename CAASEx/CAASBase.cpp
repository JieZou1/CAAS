#include "CAASBase.h"

caasBase::caasBase(const caasInput* input)
{
	startT = std::clock();	//The inspection starts here

	pixelsPerMicron = input->pixelsPerMicron;

	//Convert to gray image
	if (input->imgType == BGR)
	{
		Mat imageColor(input->imgHeight, input->imgWidth, CV_8UC3, input->imgData);
		cvtColor(imageColor, imageGray, CV_BGR2GRAY);
	}
	else if (input->imgType == BayerBGGR12)
	{
		Mat imageBayer(input->imgHeight, input->imgWidth, CV_16UC1, input->imgData);
		imageBayer.convertTo(imageBayer, CV_8UC1, 0.0625);	//0.0625 is 1/16. Convert orignal 16 bit (actually 12 bit) to 8 bit.
		cvtColor(imageBayer, imageGray, COLOR_BayerBG2GRAY);
	}

	//***** This part of testing codes proves:
	// 1. In Release version (_DEBUG is not defined), OpenCV doesn't call CV_Assert to raise exceptions for this at(.) function and possibly many other functions, due to performance considerations.
	// 2. In Debug version (_DEBUG is defined), OpenCV does call CV_Assert and raise exceptions.
	//int x = imageGray.cols, y = imageGray.rows;
	//imageGray.at<unsigned char>(y, x) = 0;
}

void caasBase::SaveGrayImage()
{
	time_t rawtime;	time(&rawtime);
	tm timeinfo; localtime_s(&timeinfo, &rawtime);
	char buffer[80];	strftime(buffer, 80, "%Y-%m-%d-%H-%M-%S", &timeinfo);
	string filename(buffer);
	filename += ".jpg";

	imwrite(filename, imageGray);
}

int caasBase::Median(Mat img)
{
	Mat hist;	int histSize = 256;	float range[] = { 0, 256 }; const float* histRange = { range };
	calcHist(&img, 1, 0, Mat(), hist, 1, &histSize, &histRange, true, false);

	int med = -1, bin = 0;
	double m = (img.rows*img.cols) / 2;

	for (int i = 0; i<256 && med<0; i++)
	{
		bin = bin + cvRound(hist.at<float>(i));
		if (bin>m && med<0) med = i;
	}
	return med;
}

void caasBase::ProjectionProfileAnalysis(Mat& profile, float& min_value, int& min_index, float& max_value, int& max_index, float values[])
{
	min_value = -1.0; max_value = -1.0;
	MatIterator_<float> it, end; float value; int i;
	for (i = 0, it = profile.begin<float>(), end = profile.end<float>(); it != end; ++it, ++i)
	{
		value = *it / 255; values[i] = value;
		if (min_value < 0) { min_value = max_value = value; min_index = max_index = 0; continue; } //first time.
		if (value < min_value) { min_value = value; min_index = i; }
		if (value > max_value) { max_value = value; max_index = i; }
	}
}

void caasBase::Gradient(int length, float values[], float gradients[])
{
	for (int i = 0; i < length; i++)
	{
		float v_prev, v_next;
		if (i == 0)
		{
			v_prev = values[i]; v_next = values[i+1];
		}
		else if (i == length - 1)
		{
			v_prev = values[i - 1]; v_next = values[i];
		}
		else
		{
			v_prev = values[i - 1]; v_next = values[i + 1];
		}

		gradients[i] = v_next - v_prev;
	}
}

void caasBase::DetectLineSegments(const Mat& image, vector<Vec4f>& lsdLines)
{
	Ptr<LineSegmentDetector> lsd = createLineSegmentDetector();
	//lsd = createLineSegmentDetector(LSD_REFINE_ADV);
	lsd->detect(image, lsdLines);

	//lsd->drawSegments(image, lsdLines);
	//imwrite("LSD.jpg", image);
}


bool SortHoGResultByWeight(const HoGResult &lhs, const HoGResult &rhs) { return lhs.weight > rhs.weight; }