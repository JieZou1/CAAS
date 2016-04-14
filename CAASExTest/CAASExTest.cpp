#include "..\CAASEx\CAASEx.h"

void main()
{
	cv::Mat image = cv::imread("/users/jie/projects/Intel/doc/SOW/CLR4-Tx/CLR4-TX-TOP_VIEW.BMP", cv::IMREAD_COLOR);

	caasInspectResult result;
	caasCLR4TxDetect(image, result);

	std::cout << "Completed!" <<std::endl;
}