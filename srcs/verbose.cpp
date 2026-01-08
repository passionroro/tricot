#include "../include/verbose.hpp"
#include <cstring>
#include <iostream>
#include <termios.h>
#include <unistd.h>

// Function to set terminal to raw mode (to capture keypresses without
// buffering)
void setRawMode() {
  struct termios t;
  tcgetattr(STDIN_FILENO, &t);          // Get current terminal attributes
  t.c_lflag &= ~(ICANON | ECHO);        // Disable canonical mode and echo
  tcsetattr(STDIN_FILENO, TCSANOW, &t); // Apply the new settings immediately
}

// Function to reset the terminal to default (normal) mode
void resetRawMode() {
  struct termios t;
  tcgetattr(STDIN_FILENO, &t);          // Get current terminal attributes
  t.c_lflag |= (ICANON | ECHO);         // Enable canonical mode and echo
  tcsetattr(STDIN_FILENO, TCSANOW, &t); // Apply the new settings immediately
}

// Function to prompt the verbose mode menu and return the selected option
VerboseOption promptVerboseMode() {
  setRawMode(); // Enable raw mode for key capturing

  const char *options[] = {"Run en mode verbose",
                           "Tester la detection des couleurs dans le header",
                           "Modifier la calibration du header"};
  int numOptions = 3;
  int selectedOption = 0;

  auto redrawMenu = [&]() {
    std::cout
        << "\033[2J\033[H"; // Clear the screen and move the cursor to top-left
    for (int i = 0; i < numOptions; ++i) {
      if (i == selectedOption) {
        std::cout << "[*] " << options[i]
                  << "\n"; // Highlight the selected option
      } else {
        std::cout << "[ ] " << options[i] << "\n";
      }
    }
  };

  redrawMenu(); // Draw the initial menu

  while (true) {
    char key;
    read(STDIN_FILENO, &key, 1); // Read a single keypress

    if (key == '\033') { // Detect escape sequences for arrow keys
      char seq[2];
      if (read(STDIN_FILENO, &seq, 2) == 2) {
        if (seq[0] == '[') {
          if (seq[1] == 'A') { // Up arrow
            selectedOption = (selectedOption - 1 + numOptions) % numOptions;
          } else if (seq[1] == 'B') { // Down arrow
            selectedOption = (selectedOption + 1) % numOptions;
          }
        }
      }
    } else if (key == '\n' || key == '\r') { // Enter key pressed
      break;
    }

    redrawMenu(); // Redraw the menu after every keypress
  }

  resetRawMode(); // Restore normal terminal mode

  // Map the selected option to the corresponding enum value and return it
  switch (selectedOption) {
  case 0:
    return RUN_VERBOSE;
  case 1:
    return TEST_HEADER_COLORS_DETECTION;
  case 2:
    return MODIFY_HEADER_CALIBRATION;
  }

  // Default return (this should never be reached)
  return RUN_VERBOSE;
}

