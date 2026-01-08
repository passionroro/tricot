#include "../include/reader.hpp"
// #include "verbose.hpp"
#include <cstring>
#include <iostream>
#include <opencv2/opencv.hpp>

int main(int argc, char **argv) {
  VerboseOption verbose;
  if (argc == 2 && std::strncmp(argv[1], "-v\0", 3) == 0) {
    verbose = promptVerboseMode();
  }

  try {
    VideoProcessor processor;
    if (verbose) {
      processor.verbose = verbose;
    }
    processor.processVideoStream();
  } catch (const cv::Exception &e) {
    std::cerr << "OpenCV error: " << e.what() << std::endl;
    return -1;
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return -1;
  }

  return 0;
}
