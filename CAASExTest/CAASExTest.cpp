#include "..\CAASEx\CAASEx.h"

#include <cv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

void main(int argc, char** argv)
{
	if (argc != 2)
	{
		std::cout << "Usage: CAASEx <Image File>" << std::endl;
		return;
	}

	String image_file = argv[1];
	cv::Mat image = cv::imread(image_file, cv::IMREAD_COLOR);

	caasImage caasImage;  caasImage.width = image.cols; caasImage.height = image.rows; caasImage.data = image.data;
	caasInspectResult caasResult;
	caasCLR4TxInspect(&caasImage, &caasResult);

	std::cout << "Completed in " << caasResult.processingTime << " seconds" << std::endl;
}