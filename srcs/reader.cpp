#include "reader.hpp"

/**
 * CONSTRUCTOR / DESTRUCTOR
 */

VideoProcessor::VideoProcessor()
{
	int x = (FRAME_WIDTH - ROI_WIDTH) / 2;
	int y = (FRAME_HEIGHT - ROI_HEIGHT) / 2;
	roi = cv::Rect(x, y, ROI_WIDTH, ROI_HEIGHT);
	isSeparatorColorSet = false;
}

/**
 * STREAM
 */

void VideoProcessor::processVideoStream()
{
	if (!openVideoStream() ||
		!loadTemplates("templates/header/instructions", headerTemplates) ||
		!loadTemplates("templates/header/border", headerBorderTemplates) ||
		!loadTemplates("templates/body", endTemplate))
	{
		return;
	}

	cv::Mat frame;
	while (true)
	{
		if (!cap.read(frame))
		{
			std::cerr << "Error: could not read frame." << std::endl;
			break;
		}

		if (colors.size() < 8)
		{
			detectTemplate(frame, headerBorderTemplates);
		}
		else
		{
			processBody(frame);
		}

		cv::rectangle(frame, roi, cv::Scalar(255, 0, 0), 2);
		cv::imshow("Video Stream", frame);

		char c = (char)cv::waitKey(25);
		if (c == 27)
		{
			break;
		}
	}

	cap.release();
	cv::destroyAllWindows();
}

bool VideoProcessor::openVideoStream()
{
	cap.open(0);
	if (!cap.isOpened())
	{
		std::cerr << "Error: could not open camera." << std::endl;
		return false;
	}

	cap.set(cv::CAP_PROP_FRAME_WIDTH, FRAME_WIDTH);
	cap.set(cv::CAP_PROP_FRAME_HEIGHT, FRAME_HEIGHT);

	std::cout << "Frame size: " << cap.get(cv::CAP_PROP_FRAME_WIDTH) << "x" << cap.get(cv::CAP_PROP_FRAME_HEIGHT) << std::endl;

	return true;
}

/**
 * TEMPLATES
 */

bool VideoProcessor::loadTemplates(const std::string &path, Template &templ)
{
	for (const auto &entry : std::filesystem::directory_iterator(path))
	{
		if (entry.path().extension() == ".jpeg" || entry.path().extension() == ".jpg" || entry.path().extension() == ".png")
		{
			cv::Mat value = cv::imread(entry.path().string(), cv::IMREAD_GRAYSCALE);
			if (value.empty())
			{
				std::cerr << "Error: Could not load image: " << entry.path() << std::endl;
				return false;
			}
			const std::string key = entry.path().stem().string();
			templ.insert(std::pair<std::string, cv::Mat>(key, value));
		}
	}

	if (templ.empty())
	{
		std::cerr << "Error: No images found in " << path << std::endl;
		return false;
	}

	std::cout << "(verbose) Loaded templates: " << path << std::endl;
	for (Template::iterator it = templ.begin(); it != templ.end(); ++it)
	{
		std::cout << it->first << "\t";
	}
	std::cout << std::endl;

	return true;
}

void VideoProcessor::detectTemplate(cv::Mat &frame, Template &templs)
{
	cv::Mat roiFrame = frame(roi);
	cv::Mat gray;
	cv::cvtColor(roiFrame, gray, cv::COLOR_BGR2GRAY);

	std::map<std::string, cv::Point> detectedLocations;

	for (const auto &[name, templ] : templs)
	{
		cv::Mat result;
		double minVal, maxVal;
		cv::Point minLoc, maxLoc;

		cv::matchTemplate(gray, templ, result, cv::TM_CCOEFF_NORMED);
		cv::minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc);

		if (maxVal > TEMPLATE_THRESHOLD)
		{
			cv::Point actualLoc(roi.x + maxLoc.x, roi.y + maxLoc.y);
			cv::rectangle(frame, actualLoc, cv::Point(actualLoc.x + templ.cols, actualLoc.y + templ.rows), cv::Scalar(0, 255, 0), 2);
			cv::putText(frame, name, actualLoc, cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 2);
			detectedLocations[name] = actualLoc;
		}
	}

	if (detectedLocations.count("header_start") && detectedLocations.count("header_end"))
	{
		cv::Point startLoc = detectedLocations["header_start"];
		cv::Point endLoc = detectedLocations["header_end"];

		int headerWidth = 120;
		int headerHeight = endLoc.y - startLoc.y - templs["header_end"].rows;
		int x = startLoc.x;
		int y = startLoc.y + templs["header_start"].rows;

		cv::Rect headerRoiRect(x, y, headerWidth, headerHeight);
		cv::rectangle(frame, headerRoiRect, cv::Scalar(0, 255, 255), 2);

		cv::Mat headerRoi = frame(headerRoiRect);
		processHeader(frame, headerRoi, x, y);
		bodyRoiPos = endLoc;
	}
}

/**
 * COLORS
 */

const int threshold = 5000;

int VideoProcessor::colorDistance(const cv::Vec3b &color1, const cv::Vec3b &color2)
{
	int dH = std::min(std::abs(color1[0] - color2[0]), 180 - std::abs(color1[0] - color2[0]));
	int dS = color1[1] - color2[1];
	int dV = color1[2] - color2[2];

	// Giving more weight to Hue differences
	return dH * dH * 4 + dS * dS + dV * dV;
}

bool VideoProcessor::areColorsSimilar(const cv::Vec3b &color1, const cv::Vec3b &color2)
{
	bool similar = false;
	int distance = colorDistance(color1, color2);

	std::cout << distance << std::endl;

	if (distance < threshold)
	{
		std::cout << "similar!" << std::endl;
		similar = true;
	}

	return similar;
}

std::string VideoProcessor::findClosestColorKey(const cv::Vec3b &dominant)
{
	std::string instruction;

	for (auto const &[key, color] : colors)
	{
		bool similar = areColorsSimilar(dominant, color);
		if (similar)
		{
			instruction = key;
			break;
		}
	}

	return instruction;
}

cv::Vec3b getDominantColor(const cv::Mat &hsvImage)
{
	int hBins = 30, sBins = 32, vBins = 32;
	int histSize[] = {hBins, sBins, vBins};
	float hRanges[] = {0, 180};
	float sRanges[] = {0, 256};
	float vRanges[] = {0, 256};
	const float *ranges[] = {hRanges, sRanges, vRanges};
	int channels[] = {0, 1, 2};

	cv::Mat hist;
	calcHist(&hsvImage, 1, channels, cv::Mat(), hist, 3, histSize, ranges, true, false);

	double maxVal;
	int maxIdx[3];
	cv::minMaxIdx(hist, 0, &maxVal, 0, maxIdx);

	return cv::Vec3b(maxIdx[0] * 180 / hBins, maxIdx[1] * 256 / sBins, maxIdx[2] * 256 / vBins);
}

/**
 * PROCESS
 */

void VideoProcessor::processHeader(cv::Mat &frame, cv::Mat &headerRoi, int x, int y)
{
	std::vector<std::string> instructions = {"+", "-", "<", ">", "[", "]", ".", ","};
	size_t colorsNb = instructions.size();

	int width = headerRoi.cols;
	int height = headerRoi.rows / colorsNb;

	cv::Mat hsvHeader;
	cv::cvtColor(headerRoi, hsvHeader, cv::COLOR_BGR2HSV);

	std::cout << "Colors detected in header: " << std::endl;
	for (int i = 0; i < colorsNb; ++i)
	{
		cv::Rect dividedHeaderRect(0, height * i, width, height);
		cv::rectangle(frame, cv::Rect(x, y + (height * i), width, height), cv::Scalar(255, 0, 255), 2);
		cv::Mat dividedHeader = hsvHeader(dividedHeaderRect);

		cv::Vec3b dominantColor = getDominantColor(dividedHeader);
		colors[instructions[i]] = dominantColor;
		std::cout << instructions[i] << " - " << colors[instructions[i]] << std::endl;
	}

	// printRBGcolors(colors);
}

cv::Vec3b prev;

void VideoProcessor::processBody(cv::Mat &frame)
{
	// TODO REVIEW POSITION (LOWER)
	// Draw body ROI
	int x = FRAME_WIDTH / 2;
	int y = bodyRoiPos.y;
	cv::Rect bodyRoiRect(x, y + 10, BODY_ROI_WIDTH, BODY_ROI_HEIGHT);
	cv::rectangle(frame, bodyRoiRect, cv::Scalar(0, 255, 255), 2);

	// Define body ROI
	cv::Mat bodyRoi = frame(bodyRoiRect);

	// Convert into HSV
	cv::Mat hsvBody;
	cv::cvtColor(bodyRoi, hsvBody, cv::COLOR_BGR2HSV);

	// Extract dominant color
	cv::Vec3b dominantColor = getDominantColor(hsvBody);

	// Initialize the separatorColor on the first time
	if (command.empty() && !isSeparatorColorSet)
	{
		separatorColor = dominantColor;
		prev = dominantColor;
		std::cout << "separator color (white):\t" << separatorColor << std::endl;
		isSeparatorColorSet = true;
		lookForColor = true;
	}

	int distance = colorDistance(dominantColor, prev);
	if (distance != 0)
	{
		std::cout << "-- verbose --" << std::endl;
		std::cout << "current color:\t" << dominantColor << std::endl;
		std::cout << "previous color:\t" << prev << std::endl;
		std::cout << "distance:\t\t" << distance << std::endl;
		prev = dominantColor;
	}

	// bool similar = areColorsSimilar(dominantColor, separatorColor);

	// if (lookForColor)
	// {
	// 	std::string instruction = findClosestColorKey(dominantColor);
	// 	if (!instruction.empty())
	// 	{
	// 		// std::cout << "(verbose) Found new color: instruction: " << result << std::endl;
	// 		command.push_back(instruction.front());
	// 		lookForColor = false;
	// 	}
	// }
	// else
	// {
	// 	// Check ending condition here

	// 	// Are we looking at the `separatorColor` ?
	// 	bool similar = areColorsSimilar(dominantColor, separatorColor);
	// 	if (!similar)
	// 	{
	// 		lookForColor = true;
	// 	}
	// }
}

/**
 * UTILS
 */

void printRBGcolors(Color &colors)
{
	Color RGBColors;
	std::cout << "    hist_colors = [" << std::endl;
	for (const auto &[instruction, hsvColor] : colors)
	{
		cv::Mat hsvMat(1, 1, CV_8UC3, hsvColor);
		cv::Mat rgbMat;
		cv::cvtColor(hsvMat, rgbMat, cv::COLOR_HSV2RGB);
		cv::Vec3b rgbColor = rgbMat.at<cv::Vec3b>(0, 0);
		RGBColors[instruction] = rgbColor;
		std::cout << "        " << rgbColor << "," << std::endl;
	}
	std::cout << "    ]" << std::endl;
}

/**
 * OVERLOADS
 */

std::ostream &operator<<(std::ostream &os, const cv::Vec3b &color)
{
	os << "H: " << static_cast<int>(color[0])
	   << ", S: " << static_cast<int>(color[1])
	   << ", V: " << static_cast<int>(color[2]);
	return os;
}