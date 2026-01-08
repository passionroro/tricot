#ifndef __VERBOSE_HPP__
#define __VERBOSE_HPP__

// Enum to define verbose options
enum VerboseOption {
  RUN_VERBOSE,
  TEST_HEADER_COLORS_DETECTION,
  MODIFY_HEADER_CALIBRATION
};

// Function to display the verbose mode menu and return the selected option
VerboseOption promptVerboseMode();

#endif // __VERBOSE_HPP__
