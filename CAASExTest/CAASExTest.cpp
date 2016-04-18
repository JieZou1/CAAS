#include "..\CAASEx\CAASEx.h"

#include <cv.hpp>
#include <iostream>
#include <fstream>

using namespace cv;
using namespace std;

cv::Mat read_raw_image(const char* filename, int width, int height)
{
	//ifstream ifs(filename, ios::binary | ios::ate);
	//ifstream::pos_type pos = ifs.tellg();

	//long length = pos;

	//char *in = new char[length];
	//ifs.seekg(0, ios::beg);
	//ifs.read(in, length);

	FILE * file = fopen(filename, "rb");
	fseek(file, 0, SEEK_END);
	int size = ftell(file);
	fclose(file);
	// Reading data to array of unsigned chars
	file = fopen(filename, "rb");
	unsigned char * in = (unsigned char *)malloc(size);
	int bytes_read = fread(in, sizeof(unsigned char), size, file);
	long n = ftell(file);
	fclose(file);

	//file = fopen("output.bin", "wb");
	//int bytes_written = fwrite(in, sizeof(unsigned char), size, file);
	//fclose(file);

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

	cv::Mat image;	caasInput* input;	caasOutput* output = new caasOutput();

	string filename = argv[1]; string ending = ".raw";
	if (filename.compare(filename.length() - ending.length(), ending.length(), ending) == 0)
	{	//RAW image
		image = image = read_raw_image(argv[1], 640, 480);
		input = new caasInput(image.cols, image.rows, BayerBGGR12, image.data);
	}
	else
	{
		image = read_bgr_image(argv[1]);
		input = new caasInput(image.cols, image.rows, BGR, image.data);
	}

	caasCLR4TxInspect(input, output);
	std::cout << "Completed in " << output->processingTime << " seconds" << std::endl;

	if (input != NULL) 
		delete input;
	delete output;
}