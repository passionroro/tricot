#include "opencv2/opencv.hpp"
#include <iostream>
#include <set>

#define FRAME_W 640
#define FRAME_H 480
#define ROI_W 16
#define ROI_H 4

using namespace cv;
using namespace std;

vector<Vec3b> detectedColors;

void printColor(const Vec3b &color)
{
	cout << "[" << static_cast<int>(color[2]) << ", "
		 << static_cast<int>(color[1]) << ", "
		 << static_cast<int>(color[0]) << "]," << endl;
}

int runColorsDetector()
{
	VideoCapture cap(0);
	if (!cap.isOpened())
	{
		cerr << "Error: Cannot open camera." << endl;
		return -1;
	}

	// set camera resolution and define ROI
	cap.set(CAP_PROP_FRAME_WIDTH, FRAME_W);
	cap.set(CAP_PROP_FRAME_HEIGHT, FRAME_H);
	Rect roi((FRAME_W - ROI_W) / 2, (FRAME_H - ROI_H) / 2, ROI_W, ROI_H);

	while (true)
	{
		Mat frame;
		cap >> frame;

		if (frame.empty())
		{
			cerr << "Error: Cannot capture frame." << endl;
			break;
		}

		Mat roiFrame = frame(roi).clone();

		// K-means clustering
		int K = 3;
		Mat labels, centers;
		Mat pixels = roiFrame.reshape(1, roiFrame.total());
		pixels.convertTo(pixels, CV_32F);
		TermCriteria criteria(TermCriteria::EPS + TermCriteria::COUNT, 10, 1.0);
		kmeans(pixels, K, labels, criteria, 1, KMEANS_PP_CENTERS, centers);

		// Convert centers to 8-bit RGB
		centers.convertTo(centers, CV_8U);

		// get unique colors
		for (int i = 0; i < centers.rows; ++i)
		{
			Vec3b color = centers.at<Vec3b>(i);
			bool similarColorFound = false;
			for (const auto &existingColor : detectedColors)
			{
				if (norm(color, existingColor) < 150)
				{ // threshold
					similarColorFound = true;
					break;
				}
			}
			if (!similarColorFound)
			{
				detectedColors.push_back(color);
			}
		}

		// display frame and ROI
		rectangle(frame, roi, Scalar(0, 255, 0), 2);
		imshow("Frame", frame);

		if (waitKey(30) == 27)
		{
			break;
		}
	}

	cap.release();
	destroyAllWindows();

	for (const auto &color : detectedColors)
	{
		printColor(color);
	}

	return 0;
}
