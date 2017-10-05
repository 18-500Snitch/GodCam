#include <opencv2/opencv.hpp>
#include "segment.h"
using namespace cv;

static uint8_t H[256];
static uint8_t S[256];
static uint8_t V[256];

void setupThreshold()
{
  for(int i = 0; i < 256; i++)
  {
    H[i] = 0;
    S[i] = 0;
    V[i] = 1;
  }

  // Threshold on red.
  for(int i = 170; i < 184; i++)
  {
    H[i] = 0x1;
  }
  for(int i = 75; i < 256; i++)
  {
    S[i] = 0x1;
  }
}

void fastThreshold(InputArray _src, OutputArray _dst)
{
  // Get the mat headers for the input array
  Mat src = _src.getMat();

  // Check that the input is of the right type
  CV_Assert(src.type() == CV_8UC3);

  // [re]create the output array so it has the right size/type
  _dst.create(src.size(),CV_8UC1);

  // Get the mat header for the output array
  Mat dst = _dst.getMat();

  int channels = src.channels();
  int nRows = src.rows;
  int nCols = src.cols;
  if(src.isContinuous() && dst.isContinuous())
  {
    nCols *= nRows;
    nRows = 1;
  }

  // Threshold the image in the HSV space using the lookup table method
  uchar* pSrc;
  uchar* pDst;
  for(int i = 0; i < nRows; i++)
  {
    pSrc = src.ptr<uchar>(i);
    pDst = dst.ptr<uchar>(i);
    for(int j = 0; j < nCols; j++)
    {
      int index = j * channels;
      pDst[j] = (H[pSrc[index]] & S[pSrc[index+1]] & V[pSrc[index+2]]) * 255;
    }
  }
}
