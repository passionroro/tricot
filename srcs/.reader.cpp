#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

#define CAMERA_INDEX 0
#define ROI_WIDTH 128
#define ROI_HEIGHT 16
#define NUM_COLORS 1024
#define K_CLUSTERS 3

void printColor(const Scalar& color) {
	cout << color[0] << ", " << color[1] << ", " << color[2] << endl;
}

int main() {
	// Open camera
	VideoCapture cap(CAMERA_INDEX);

	if (!cap.isOpened()) {
		cerr << "Error: Unable to open camera." << endl;
		return -1;
	}

	cap.set(CAP_PROP_FRAME_WIDTH, ROI_WIDTH);
	cap.set(CAP_PROP_FRAME_HEIGHT, ROI_HEIGHT);

	vector<Scalar>	colors;

	while (true) {
		Mat frame;
		cap >> frame;
		if (frame.empty()) {
			cerr << "Error: Unable to capture frame." << endl;
			break;
		}

		const int x = (frame.cols - ROI_WIDTH) / 2;
		const int y = (frame.rows - ROI_HEIGHT) / 2;
		Rect roi(x, y, ROI_WIDTH, ROI_HEIGHT);

		Mat _roi = frame(roi).clone();
		Mat reshaped = _roi.reshape(1, ROI_HEIGHT * ROI_WIDTH);
		reshaped.convertTo(reshaped, CV_32FC3);

		Mat centers, labels;
		TermCriteria criteria(TermCriteria::EPS + TermCriteria::COUNT, 10, 1.0);
		kmeans(reshaped, K_CLUSTERS, labels, criteria, 10, KMEANS_RANDOM_CENTERS, centers);

		for (int i = 0; i < K_CLUSTERS; i++) {
			Scalar color = centers.at<Vec3f>(i);
			bool knownColor = true;
			for (const auto& detectedColor : colors) {
				if (norm(detectedColor, color) < 50) {
					knownColor = false;
					break;
				}
			}
			if (knownColor) {
				printColor(color);
				colors.push_back(color);
			}
		}

		if (colors.size() >= NUM_COLORS) {
			break;
		}

		imshow("ROI", _roi);

		if (waitKey(30) >= 0) {
			break;
		}
	}

	cap.release();
	destroyAllWindows();

	return 0;
}

