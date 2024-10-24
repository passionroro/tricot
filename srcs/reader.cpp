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

	printVerbose(frame, "Looking for the header !!");

	for (const auto &[name, templ] : templs)
	{
		cv::Mat result;
		double minVal, maxVal;
		cv::Point minLoc, maxLoc;

		cv::matchTemplate(gray, templ, result, cv::TM_CCOEFF_NORMED);
		cv::minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc);

		if (maxVal > TEMPLATE_THRESHOLD)
		{
			printVerbose(frame, "Found a part of the header !");
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

		printVerbose(frame, "Found the whole header !");

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

const int threshold = 500;

int VideoProcessor::colorDistanceBGR(const cv::Vec3b &color1, const cv::Vec3b &color2)
{
	int dB = color1[0] - color2[0];
	int dG = color1[1] - color2[1];
	int dR = color1[2] - color2[2];

	return dB * dB + dG * dG + dR * dR;
}

bool VideoProcessor::areColorsSimilar(const cv::Vec3b &color1, const cv::Vec3b &color2)
{
	return (colorDistanceBGR(color1, color2) < threshold ? true : false);
}

std::string VideoProcessor::findClosestColorKey(const cv::Vec3b &dominant)
{
	std::string instruction;

	std::cout << "-- Iterating through all the colors --" << std::endl;
	for (auto const &[key, color] : colors)
	{
		bool similar = areColorsSimilar(dominant, color);
		std::cout << "Distance between " << dominant << " and " << color << " is " << colorDistanceBGR(dominant, color) << std::endl;
		if (similar)
		{
			instruction = key;
			break;
		}
	}

	return instruction;
}

cv::Vec3b VideoProcessor::getDominantColorBGR_KMeans(const cv::Mat &image, int k)
{
	CV_Assert(image.channels() == 3);

	// Ensure the image is continuous
	cv::Mat continuousImage;
	if (!image.isContinuous())
	{
		continuousImage = image.clone();
	}
	else
	{
		continuousImage = image;
	}

	// Reshape the image to a 2D array of pixels
	cv::Mat pixels = continuousImage.reshape(1, continuousImage.total());
	pixels.convertTo(pixels, CV_32F);

	// Define criteria and apply kmeans
	cv::TermCriteria criteria(cv::TermCriteria::EPS + cv::TermCriteria::MAX_ITER, 100, 0.2);
	cv::Mat labels, centers;
	cv::kmeans(pixels, k, labels, criteria, 10, cv::KMEANS_RANDOM_CENTERS, centers);

	// Find the cluster with the most pixels
	std::vector<int> clusterSizes(k, 0);
	for (int i = 0; i < labels.rows; ++i)
	{
		clusterSizes[labels.at<int>(i)]++;
	}

	int dominantCluster = std::distance(clusterSizes.begin(),
										std::max_element(clusterSizes.begin(), clusterSizes.end()));


	// Return the color of the dominant cluster
	cv::Vec3f dominantColor = centers.row(dominantCluster);

	// To do : add some verbose when the dominantColor is 0, 255, 255 and 
	// print all relevant information about the KMeans

	return cv::Vec3b(cv::saturate_cast<uchar>(dominantColor[0]),
					 cv::saturate_cast<uchar>(dominantColor[1]),
					 cv::saturate_cast<uchar>(dominantColor[2]));
}

cv::Vec3b VideoProcessor::getDominantColorBGR(const cv::Mat &image)
{
	int rBins = 32, gBins = 32, bBins = 32;
	int histSize[] = {rBins, gBins, bBins};
	float rRanges[] = {0, 256};
	float gRanges[] = {0, 256};
	float bRanges[] = {0, 256};
	const float *ranges[] = {rRanges, gRanges, bRanges};
	int channels[] = {0, 1, 2};

	cv::Mat hist;
	calcHist(&image, 1, channels, cv::Mat(), hist, 3, histSize, ranges, true, false);

	double maxVal;
	int maxIdx[3];
	cv::minMaxIdx(hist, 0, &maxVal, 0, maxIdx);

	return cv::Vec3b(maxIdx[0] * 256 / bBins, maxIdx[1] * 256 / gBins, maxIdx[2] * 256 / rBins);
}

/**
 * PROCESS
 */
#include <ctime>
void VideoProcessor::processHeader(cv::Mat &frame, cv::Mat &headerRoi, int x, int y)
{
	std::vector<std::string> instructions = {"+", "-", "<", ">", "[", "]", ".", ","};
	size_t colorsNb = instructions.size();

	int width = headerRoi.cols;
	int height = headerRoi.rows / colorsNb;

	printVerbose(frame, "Detecing the colors in the header");

	for (int i = 0; i < colorsNb; ++i)
	{
		cv::Rect dividedHeaderRect(0, height * i, width, height);
		cv::rectangle(frame, cv::Rect(x, y + (height * i), width, height), cv::Scalar(255, 0, 255), 2);
		cv::Mat dividedHeader = headerRoi(dividedHeaderRect);

		cv::Vec3b dominantColor = getDominantColorBGR_KMeans(dividedHeader);
		colors[instructions[i]] = dominantColor;

		// Verbose : show detected colors
		cv::Mat comparisonMat(height, width * 2, dividedHeader.type());
		dividedHeader.copyTo(comparisonMat(cv::Rect(0, 0, width, height)));
		comparisonMat(cv::Rect(width, 0, width, height)) = cv::Scalar(dominantColor[0], dominantColor[1], dominantColor[2]);
		const std::string name = "header_section_" + std::to_string(i);
		// Show the images
		cv::imshow(name, comparisonMat);
		cv::moveWindow(name, 0, i * 1.5 * height);
		// Save the image
		// saveImage(name, comparisonMat);
	}
}

void VideoProcessor::processBody(cv::Mat &frame)
{
	// Draw body ROI
	int x = FRAME_WIDTH / 2;
	int y = bodyRoiPos.y + 16;
	cv::Rect bodyRoiRect(x, y, BODY_ROI_WIDTH, BODY_ROI_HEIGHT);
	cv::rectangle(frame, bodyRoiRect, cv::Scalar(0, 255, 255), 2);

	printVerbose(frame, "Finished. Now ready to interpret the detected colors.");

	// Define body ROI
	cv::Mat bodyRoi = frame(bodyRoiRect);

	// Verbose: magnify the bodyRoi
	verboseMagnifyImage(bodyRoi);

	// RGB Dominant Color
	cv::Vec3b dominantColorBGR = getDominantColorBGR_KMeans(bodyRoi);

	// Initialize the separatorColor on the first time
	if (command.empty() && !isSeparatorColorSet)
	{
		// Ca c'est du gros bricolage mdr
		while (areColorsSimilar(dominantColorBGR, cv::Vec3b(0, 255, 255)))
		{
			dominantColorBGR = getDominantColorBGR_KMeans(bodyRoi);
		}
		separatorColorBGR = dominantColorBGR;
		isSeparatorColorSet = true;
		lookForColor = true;
	}

	// Verbose: draw separatorColor on the top-right of the screen
	const int colorWindowWidth = 128;
	const int colorWindowHeight = 64;
	cv::Mat colorWindow(colorWindowHeight, colorWindowWidth, CV_8UC3, separatorColorBGR);
	const std::string windowName = "Separator Color";
	cv::namedWindow(windowName, cv::WINDOW_NORMAL);
	cv::resizeWindow(windowName, colorWindowWidth, colorWindowHeight);
	cv::moveWindow(windowName, 1024, 0);
	cv::imshow(windowName, colorWindow);

	if (lookForColor)
	{
		std::string instruction = findClosestColorKey(dominantColorBGR);
		if (!instruction.empty())
		{
			std::cout << "(verbose) Found new color: instruction: " << instruction.front() << std::endl;
			command.push_back(instruction.front());
			lookForColor = false;
		}
	}
	else
	{
		// Check ending condition here

		// Are we looking at the `separatorColor` ?
		std::cout << "Now looking for separator color:" 
			<< "\nseparator: " << separatorColorBGR 
			<< "\ncurrent: " << dominantColorBGR << std::endl;
		bool similar = areColorsSimilar(dominantColorBGR, separatorColorBGR);
		if (similar)
		{
			std::cout << "Apparently, we are currently looking at a color similar to separator color !"
				<< "\ndistance: " << colorDistanceBGR(dominantColorBGR, separatorColorBGR)
				<< "\nseparator: " << separatorColorBGR 
				<< "\ncurrent: " << dominantColorBGR << std::endl;
			lookForColor = true;
		}
	}
}

/**
 * UTILS
 */

void VideoProcessor::verboseMagnifyImage(const cv::Mat &img, int n)
{
	cv::Mat magnified;
	cv::resize(img, magnified, cv::Size(), n, n, cv::INTER_NEAREST);

	const std::string name = "Magnified Body ROI";
	cv::moveWindow(name, 1024, 256);
	cv::imshow(name, magnified);
}

void VideoProcessor::printVerbose(cv::Mat &frame, const std::string &text)
{
	const int x = 10;
	const int y = 10;
	const int height = 64;
	const int width = FRAME_WIDTH - (x + y);

	cv::rectangle(frame, cv::Rect(x, y, width, height), cv::Scalar(255, 255, 255), 3);
	cv::rectangle(frame, cv::Rect(x, y, width, height), cv::Scalar(0, 0, 0), -1);
	cv::Point textOrg(x + 5, y + 40);
	cv::putText(frame, text, textOrg, cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 255, 255), 2);
}

void VideoProcessor::saveImage(const std::string &name, cv::Mat &img)
{
	std::time_t result = std::time(nullptr);
	std::stringstream ss;
	ss << std::put_time(std::localtime(&result), "%Y%m%d_%H%M%S_");
	std::string timestamp = ss.str();

	std::string filename = timestamp + name + ".png";
	cv::imwrite(filename, img);
}

/**
 * OVERLOADS
 */

std::ostream &operator<<(std::ostream &os, const cv::Vec3b &color)
{
	os << static_cast<int>(color[0])
	   << ", " << static_cast<int>(color[1])
	   << ", " << static_cast<int>(color[2]);
	return os;
}