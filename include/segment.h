#ifndef SEGMENT
#define SEGMENT

#include <opencv2/opencv.hpp>

using namespace cv;

void setupThreshold();
void fastThreshold(InputArray _src, OutputArray _dst);

#endif
