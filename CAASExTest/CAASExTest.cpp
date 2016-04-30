#include "..\CAASEx\CAASEx.h"

#include <cv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

cv::Mat read_raw_image(const char* filename, int width, int height)
{
	FILE * file = fopen(filename, "rb");
	fseek(file, 0, SEEK_END);
	long size = ftell(file);
	fclose(file);

	// Reading data to array of unsigned chars
	file = fopen(filename, "rb");
	unsigned char * in = (unsigned char *)malloc(size);
	long bytes_read = (long)fread(in, sizeof(unsigned char), size, file);
	fclose(file);

	cv::Mat img(height, width, CV_16UC1, in);
	cv::Mat image = img.clone();

	free(in);

	return image;
}

cv::Mat read_bgr_image(char* filename)
{
	return cv::imread(filename, cv::IMREAD_COLOR);
}

void main(int argc, char** argv)
{
	if (argc != 2)
	{
		std::cout << "Usage: CAASEx <Image File>" << std::endl;
		return;
	}

	caasInput* input;	caasOutput* output = new caasOutput();

	cv::Mat image;  string filename = argv[1]; string ending = ".raw";
	if (filename.compare(filename.length() - ending.length(), ending.length(), ending) == 0)
	{	//RAW image
		image = read_raw_image(argv[1], 640, 480);
		input = new caasInput(image.cols, image.rows, BayerBGGR12, image.data, 0.75);
	}
	else
	{
		image = read_bgr_image(argv[1]);
		input = new caasInput(image.cols, image.rows, BGR, image.data, 0.75);
	}

	caasCLR4TxInspect(input, output);

	if (output->targetRightEdge == -1 || output->targetLeftEdge == -1 || output->isolatorRightEdge == -1)
	{
		std::cout << "Error in Inspection " << std::endl;
	}
	else
	{
		std::cout << "Distance In Pixels " << output->distanceInPixels << std::endl;
		std::cout << "Distance In Microns " << output->distanceInMicrons << std::endl;
		std::cout << "Angle in degrees " << output->isolatorAngle << std::endl;
		std::cout << "Completed in " << output->processingTime << " seconds" << std::endl;

		Mat imageSmall; resize(image, imageSmall, Size(image.cols / 4, image.rows / 4));
		line(imageSmall, Point(output->isolatorRightEdge / 4, 0), Point(output->isolatorRightEdge / 4, image.cols / 4), 255, 3, 8);
		line(imageSmall, Point(output->targetLeftEdge / 4, 0), Point(output->targetLeftEdge / 4, image.cols / 4), 255, 3, 8);
		line(imageSmall, Point(output->targetRightEdge / 4, 0), Point(output->targetRightEdge / 4, image.cols / 4), 255, 3, 8);
		imshow("result", imageSmall);
		imwrite("result.jpg", imageSmall);
		waitKey(0);
	}

	if (input != NULL) 		delete input;
	delete output;
}