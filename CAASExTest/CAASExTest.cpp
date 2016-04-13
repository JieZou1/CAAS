#include "..\CAASEx\CAASEx.h"

#include <iostream>

void main()
{
	cv::Mat image = cv::imread("/users/jie/projects/Intel/doc/SOW/CLR4-Tx/CLR4-TX-TOP_VIEW.BMP", cv::IMREAD_COLOR);

	std::cout << "Completed!";
}