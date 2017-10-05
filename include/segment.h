#ifndef SEGMENT
#define SEGMENT

#include <opencv2/opencv.hpp>

void setupThreshold();
void fastThreshold(cv::InputArray _src, cv::OutputArray _dst);

#endif
