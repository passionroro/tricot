#ifndef __READER_HPP__
#define __READER_HPP__

#include "verbose.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

#define FRAME_WIDTH 1920
#define FRAME_HEIGHT 1080
#define ROI_WIDTH 512
#define ROI_HEIGHT 1024
#define BODY_ROI_WIDTH 48
#define BODY_ROI_HEIGHT 12
#define TEMPLATE_THRESHOLD 0.8

typedef std::map<std::string, cv::Mat> Template;
typedef std::unordered_map<std::string, cv::Vec3b> Color;

class VideoProcessor {
public:
  VideoProcessor();
  ~VideoProcessor() = default;
  void processVideoStream();
  VerboseOption verbose;

private:
  bool read = true;
  cv::VideoCapture cap;
  cv::Rect roi;
  Template headerTemplates;
  Template headerBorderTemplates;
  Template endTemplate;
  Color colors;

  cv::Point bodyRoiPos;
  cv::Vec3b separatorColorBGR;
  bool isSeparatorColorSet;
  std::string command;
  bool lookForColor;

  bool openVideoStream();
  void processHeader(cv::Mat &frame, cv::Mat &headerRoi, int x, int y);
  void processBody(cv::Mat &frame);

  bool loadTemplates(const std::string &path, Template &templ);
  void detectTemplate(cv::Mat &frame, Template &templs);

  bool areColorsSimilar(const cv::Vec3b &color1, const cv::Vec3b &color2);
  std::string findClosestColorKey(const cv::Vec3b &dominant);
  int colorDistanceBGR(const cv::Vec3b &color1, const cv::Vec3b &color2);
  cv::Vec3b getDominantColorBGR(const cv::Mat &image);
  cv::Vec3b getDominantColorBGR_KMeans(const cv::Mat &image, int k = 4);

  void printVerbose(cv::Mat &frame, const std::string &text);
  void verboseMagnifyImage(const cv::Mat &img, int n = 4);
  void saveImage(const std::string &name, cv::Mat &img,
                 const std::string &path);
  void saveSeparatorColor(cv::Mat &roi, int width, int height);

      enum class AdjustmentMode {
        Brightness,
        Contrast
      };
  static constexpr const char *BRIGHTNESS_FILE =
      "assets/calibration/brightness.dat";
  static constexpr const char *CONTRAST_FILE =
      "assets/calibration/contrast.dat";
  static constexpr double ADJUSTMENT_STEP = 0.1;
  double currentBrightness = 1.0;
  double currentContrast = 1.0;
  AdjustmentMode currentMode = AdjustmentMode::Brightness;
  template <typename T>
  void saveValue(const char *filename, const T &value) const {
    std::filesystem::create_directories(
        std::filesystem::path(filename).parent_path());

    std::ofstream file(filename, std::ios::binary);
    if (file.is_open()) {
      file.write(reinterpret_cast<const char *>(&value), sizeof(value));
    }
  }
  template <typename T> void loadValue(const char *filename, T &value) {
    std::ifstream file(filename, std::ios::binary);
    if (file.is_open()) {
      file.read(reinterpret_cast<char *>(&value), sizeof(value));
    }
  }
  void saveCurrentAdjustments() const;
  void loadAdjustments();
  void adjustFrame(cv::Mat &frame) const;
  bool handleCalibrationControl(const int &key, cv::Mat &frame);
  void printVerboseCalibration(cv::Mat &frame);
};

std::ostream &operator<<(std::ostream &os, const cv::Vec3b &color);

#endif // __READER_HPP__