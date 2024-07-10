#include "reader.hpp"
#include <iostream>

int main()
{
	try
	{
		VideoProcessor processor;
		processor.processVideoStream();
	}
	catch (const cv::Exception &e)
	{
		std::cerr << "OpenCV error: " << e.what() << std::endl;
		return -1;
	}
	catch (const std::exception &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return -1;
	}

	return 0;
}
