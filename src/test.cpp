#include <stdio.h>
#include <opencv2/opencv.hpp>
#include "segment.h"

#define CAMFPS 30
#define CAMWIDTH 848
#define CAMHEIGHT 480

using namespace cv;
using namespace std;

int main(int argc, char** argv )
{
  setupThreshold();
  VideoCapture cap(0);
  if(!cap.isOpened())
  return -1;

  cap.set(CAP_PROP_FPS,CAMFPS);
  cap.set(CAP_PROP_FRAME_WIDTH,CAMWIDTH);
  cap.set(CAP_PROP_FRAME_HEIGHT,CAMHEIGHT);
  cap.set(CV_CAP_PROP_FOURCC ,CV_FOURCC('M', 'J', 'P', 'G') );
  cap.set(CAP_PROP_MODE,CAP_MODE_BGR);

  int ex = static_cast<int>(cap.get(CAP_PROP_FOURCC));
  char EXT1[] = {
    (char)(ex & 0XFF),
    (char)((ex & 0XFF00) >> 8),
    (char)((ex & 0XFF0000) >> 16),
    (char)((ex & 0XFF000000) >> 24),
    0
  };

  printf("Cam FPS: %f  Res: %f x %f Format: %s\n",
  cap.get(CAP_PROP_FPS),
  cap.get(CAP_PROP_FRAME_WIDTH),
  cap.get(CAP_PROP_FRAME_HEIGHT),
  EXT1);

  Mat frame;
  Mat framePipe;
  Mat frameHSV;
  Mat threshold;
  vector<vector<Point> > contours;
  vector<Vec4i> hirerarchy;
  RNG rng(12345);

  cap >> frame;
  printf("type: %d, width: %d, height: %d \n",
  frame.type(),
  frame.cols,
  frame.rows);
  namedWindow("camera",1);
  namedWindow("threshold",1);
  for(;;)
  {
    if(cap.grab())
    {
      printf("%f\n",cap.get(CAP_PROP_POS_MSEC));
      if(cap.retrieve(frame))
      {
        bilateralFilter(frame,framePipe,6,100,100);
        bilateralFilter(framePipe,frame,6,100,100);
        bilateralFilter(frame,framePipe,6,100,100);
        cvtColor(framePipe,frameHSV,COLOR_BGR2HSV);
        fastThreshold(frameHSV, threshold);
        findContours(threshold,contours,hirerarchy,RETR_TREE,CHAIN_APPROX_SIMPLE,Point(0,0));
        vector<vector<Point> > contours_poly( contours.size() );
        vector<Rect> boundRect( contours.size() );
        for( size_t i = 0; i < contours.size(); i++ )
        {
          approxPolyDP( Mat(contours[i]), contours_poly[i], 3, true );
          boundRect[i] = boundingRect( Mat(contours_poly[i]) );
        }
        for( size_t i = 0; i< contours.size(); i++ )
        {
          Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
          if(boundRect[i].area() >=10000 && hirerarchy[i][3] < 0)
          {
            rectangle( framePipe, boundRect[i].tl(), boundRect[i].br(), color, 2, 8, 0 );
          }
        }
        imshow("camera",framePipe);
        imshow("threshold",threshold);
      }
    }
    waitKey(1);
  }
  return 0;
}
