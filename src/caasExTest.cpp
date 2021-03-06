#include "caasEx.h"

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

cv::Mat read_bgr_image(const char* filename)
{
	return cv::imread(filename, cv::IMREAD_COLOR);
}

void test_clr4tx(string filename)
{
	caasInput* input;	caasCLR4TxOutput* output = new caasCLR4TxOutput();

	cv::Mat image;  string ending = ".raw";
	if (filename.compare(filename.length() - ending.length(), ending.length(), ending) == 0)
	{	//RAW image
		image = read_raw_image(filename.c_str(), 640, 480);
		input = new caasInput(image.cols, image.rows, BayerBGGR12, image.data, 0.75);
	}
	else
	{
		image = read_bgr_image(filename.c_str());
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
		std::cout << "Distance Between Lines In Microns " << output->distanceBetweenLinesinMicrons << std::endl;
		std::cout << "Completed in " << output->processingTime << " seconds" << std::endl;

		Mat imageSmall; resize(image, imageSmall, Size(image.cols / 4, image.rows / 4));
		line(imageSmall, Point(output->isolatorRightEdge / 4, 0), Point(output->isolatorRightEdge / 4, image.cols / 4), 255, 3, 8);
		line(imageSmall, Point(output->targetLeftEdge / 4, 0), Point(output->targetLeftEdge / 4, image.cols / 4), 255, 3, 8);
		line(imageSmall, Point(output->targetRightEdge / 4, 0), Point(output->targetRightEdge / 4, image.cols / 4), 255, 3, 8);
		line(imageSmall, Point(0, output->isolatorMiddleY / 4), Point(image.cols / 4, output->isolatorMiddleY / 4), 255, 3, 8);
		line(imageSmall, Point(0, output->baseMiddleY / 4), Point(image.cols / 4, output->baseMiddleY / 4), Scalar(0, 0, 255), 3, 8);
		imshow("result", imageSmall);
		imwrite("result.jpg", imageSmall);
		waitKey(0);
	}

	if (input != NULL) 		delete input;
	delete output;
}

void test_psm4tx(string filename)
{
	caasInput* input;	caasPSM4TxOutput* output = new caasPSM4TxOutput();

	cv::Mat image;  string ending = ".raw";
	if (filename.compare(filename.length() - ending.length(), ending.length(), ending) == 0)
	{	//RAW image
		image = read_raw_image(filename.c_str(), 640, 480);
		input = new caasInput(image.cols, image.rows, BayerBGGR12, image.data, 0.75);
	}
	else
	{
		image = read_bgr_image(filename.c_str());
		input = new caasInput(image.cols, image.rows, BGR, image.data, 0.75);
	}

	caasPSM4TxInspect(input, output);

	if (output->aperture2ndMidY == -1)
	{
		std::cout << "Error in Inspection " << std::endl;
	}
	else
	{
		std::cout << "Horizontal Distance In Pixels " << output->horDistanceInPixels << std::endl;
		std::cout << "Horizontal Distance In Microns " << output->horDistanceInMicrons << std::endl;
		std::cout << "Vertical Distance In Pixels " << output->verDistanceInPixels << std::endl;
		std::cout << "Vertical Distance In Microns " << output->verDistanceInMicrons << std::endl;
		std::cout << "Angle in degrees " << output->angle << std::endl;
		std::cout << "Completed in " << output->processingTime << " seconds" << std::endl;

		Mat imageSmall; resize(image, imageSmall, Size(image.cols / 4, image.rows / 4));
		line(imageSmall, Point(output->isolatorLeftEdge / 4, 0), Point(output->isolatorLeftEdge / 4, image.cols / 4), Scalar(0, 255, 0), 3, 8); //Green
		line(imageSmall, Point(output->arrayblockRightEdge / 4, 0), Point(output->arrayblockRightEdge / 4, image.cols / 4), Scalar(0, 0, 255), 3, 8); //Red
		line(imageSmall, Point(0, output->arrayblock1stMidY / 4), Point(image.cols / 4, output->arrayblock1stMidY / 4), Scalar(0, 0, 255), 3, 8); //Red
		line(imageSmall, Point(0, output->aperture2ndMidY / 4), Point(image.cols / 4, output->aperture2ndMidY / 4), Scalar(0, 255, 0), 3, 8);	//Green
		imshow("result", imageSmall);
		imwrite("result.jpg", imageSmall);
		waitKey(0);
	}

	if (input != NULL) 		delete input;
	delete output;
}

void main(int argc, char** argv)
{
	if (argc != 3)
	{
		std::cout << "Usage: CAASEx <Platform> <Image File>" << std::endl;
		std::cout << "       Platform: CLR4Tx, PSM4Tx " << std::endl;
		return;
	}

	string platform(argv[1]), filename(argv[2]);

	if (platform == "CLR4Tx")
		test_clr4tx(filename);
	else if (platform == "PSM4Tx")
		test_psm4tx(filename);
	else
		std::cout << "Invalid Platform Type" << std::endl;


}