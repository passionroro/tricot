#ifndef __READER_HPP__
#define __READER_HPP__

#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <filesystem>

#define FRAME_WIDTH 1920
#define FRAME_HEIGHT 1080
#define ROI_WIDTH 512
#define ROI_HEIGHT 512
#define BODY_ROI_WIDTH 48
#define BODY_ROI_HEIGHT 12
#define TEMPLATE_THRESHOLD 0.8

typedef std::map<std::string, cv::Mat> Template;
typedef std::unordered_map<std::string, cv::Vec3b> Color;

class VideoProcessor
{
public:
	VideoProcessor();
	~VideoProcessor() = default;
	void processVideoStream();

private:
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
	void saveImage(const std::string &name, cv::Mat &img);


};

std::ostream &operator<<(std::ostream &os, const cv::Vec3b &color);

#endif // __READER_HPP__