#include "..\CAASEx\CAASEx.h"

void main(int argc, char** argv)
{
	if (argc != 2)
	{
		std::cout << "Usage: CAASEx <Image File>" << std::endl;
		return;
	}

	String image_file = argv[1];
	cv::Mat image = cv::imread(image_file, cv::IMREAD_COLOR);

	caasInspectResult result;
	caasCLR4TxInspect(image, &result);

	std::cout << "Completed in " << result.processingTime << " seconds" <<std::endl;
}