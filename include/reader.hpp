#ifndef READER_HPP
#define READER_HPP

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
#define BODY_ROI_WIDTH 64
#define BODY_ROI_HEIGHT 16
#define TEMPLATE_THRESHOLD 0.8

typedef std::map<std::string, cv::Mat> Template;
typedef std::unordered_map<std::string, cv::Vec3b> Color;

class VideoProcessor
{
private:
	cv::VideoCapture cap;
	cv::Rect roi;
	Template headerTemplates;
	Template headerBorderTemplates;
	Template endTemplate;
	Color colors;

	cv::Point bodyRoiPos;
	cv::Vec3b separatorColor;
	bool isSeparatorColorSet;
	std::string command;
	bool		lookForColor;

	bool openVideoStream();
	bool loadTemplates(const std::string &path, Template &templ);
	void detectTemplate(cv::Mat &frame, Template &templs);
	void processHeader(cv::Mat &frame, cv::Mat &headerRoi, int x, int y);
	void processBody(cv::Mat &frame);

	bool areColorsSimilar(const cv::Vec3b &color1, const cv::Vec3b &color2);
	std::string findClosestColorKey(const cv::Vec3b &dominant);

	int colorDistance(const cv::Vec3b &color1, const cv::Vec3b &color2);

public:
	VideoProcessor();
	~VideoProcessor() = default;

	void processVideoStream();
};

std::ostream &operator<<(std::ostream &os, const cv::Vec3b &color);

#endif // READER_HPP
