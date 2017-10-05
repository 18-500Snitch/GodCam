#include <stdio.h>
#include <opencv2/opencv.hpp>
#include "segment.h"

#define CAMFPS 60
#define CAMWIDTH 640
#define CAMHEIGHT 480

using namespace cv;
int main(int argc, char** argv )
{
  setupThreshold();
  VideoCapture cap1(1);
  VideoCapture cap2(2);
  if(!cap1.isOpened() || !cap2.isOpened())
    return -1;

  cap1.set(CAP_PROP_FPS,CAMFPS);
  cap1.set(CAP_PROP_FRAME_WIDTH,CAMWIDTH);
  cap1.set(CAP_PROP_FRAME_HEIGHT,CAMHEIGHT);
  cap1.set(CV_CAP_PROP_FOURCC ,CV_FOURCC('M', 'J', 'P', 'G') );
  //cap1.set(CAP_PROP_MODE,CAP_MODE_BGR);

  cap2.set(CAP_PROP_FPS,CAMFPS);
  cap2.set(CAP_PROP_FRAME_WIDTH,CAMWIDTH);
  cap2.set(CAP_PROP_FRAME_HEIGHT,CAMHEIGHT);
  cap2.set(CV_CAP_PROP_FOURCC ,CV_FOURCC('M', 'J', 'P', 'G') );
  //cap2.set(CAP_PROP_MODE,CAP_MODE_BGR);

  int ex = static_cast<int>(cap1.get(CAP_PROP_FOURCC));
  char EXT1[] = {
    (char)(ex & 0XFF),
    (char)((ex & 0XFF00) >> 8),
    (char)((ex & 0XFF0000) >> 16),
    (char)((ex & 0XFF000000) >> 24),
    0};

  printf("Cam 1 FPS: %f  Res: %f x %f Format: %s\n",
      cap1.get(CAP_PROP_FPS),
      cap1.get(CAP_PROP_FRAME_WIDTH),
      cap1.get(CAP_PROP_FRAME_HEIGHT),
      EXT1);


  ex = static_cast<int>(cap1.get(CAP_PROP_FOURCC));
  char EXT2[] = {
    (char)(ex & 0XFF),
    (char)((ex & 0XFF00) >> 8),
    (char)((ex & 0XFF0000) >> 16),
    (char)((ex & 0XFF000000) >> 24),
    0};

  printf("Cam 2 FPS: %f  Res: %f x %f Format: %s\n",
      cap2.get(CAP_PROP_FPS),
      cap2.get(CAP_PROP_FRAME_WIDTH),
      cap2.get(CAP_PROP_FRAME_HEIGHT),
      EXT2);

  Mat frameTest;
  cap1 >> frameTest;
  printf("type: %d, width: %d, height: %d \n",
      frameTest.type(),
      frameTest.cols,
      frameTest.rows);
  namedWindow("camera",1);
  namedWindow("threshold",1);
  for(;;)
  {
    Mat frame1;
    Mat frame2;
    Mat frame1HSV;
    Mat frame2HSV;
    Mat threshold1;
    Mat threshold2;
    cap1 >> frame1;
    cap2 >> frame2;
    cvtColor(frame1,frame1HSV,COLOR_BGR2HSV);
    cvtColor(frame2,frame2HSV,COLOR_BGR2HSV);
    int rows = frame1.rows;
    int cols = frame1.cols;
    fastThreshold(frame1HSV, threshold1);
    fastThreshold(frame2HSV, threshold2);
    Mat combinedFrame(rows * 2, cols,frame1.type());
    Mat combinedThreshold(rows * 2, cols,CV_8UC1);
    frame1.copyTo(combinedFrame.rowRange(0,rows).colRange(0,cols));
    frame2.copyTo(combinedFrame.rowRange(rows, 2*rows).colRange(0,cols));

    threshold1.copyTo(combinedThreshold.rowRange(0,rows).colRange(0,cols));
    threshold2.copyTo(combinedThreshold.rowRange(rows, 2*rows).colRange(0,cols));
    imshow("camera",combinedFrame);
    imshow("threshold",combinedThreshold);
    if(waitKey(1000/CAMFPS) >= 0) break;
  }
  return 0;
}
