#include <stdio.h>
#include <opencv2/opencv.hpp>
#include "segment.h"
#include "syncCams.h"


#define CAMFPS 30
#define CAMWIDTH 848
#define CAMHEIGHT 480

using namespace cv;
using namespace std;

void runThreshold(int index)
{
  setupThreshold();
  VideoCapture cap(index);
  if(!cap.isOpened())
    return;

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
}

void runSyncTest()
{
  // Init the synced camera class
  SyncCams syncedCams(vector<int>{1,3});

  if(!syncedCams.isOpened())
    return;

  syncedCams.set(CAP_PROP_FPS,30);
  syncedCams.set(CAP_PROP_FRAME_WIDTH,640);
  syncedCams.set(CAP_PROP_FRAME_HEIGHT,480);
  syncedCams.set(CV_CAP_PROP_FOURCC ,CV_FOURCC('M', 'J', 'P', 'G') );
  syncedCams.set(CAP_PROP_MODE,CAP_MODE_BGR);

  // Init the video writer
  VideoWriter writer;
  int codec = CV_FOURCC('M', 'J', 'P', 'G');  // select desired codec (must be available at runtime)
  double fps = 30;                          // framerate of the created video stream
  string filename = "./unsynced.avi";             // name of the output video file
  writer.open(filename, codec, fps, Size(640,480*2), true);
  // check if we succeeded
  if (!writer.isOpened()) {
    cerr << "Could not open the output video file for write\n";
    return;
  }

  cout << syncedCams.startCapturing();
  vector<Mat> output(2);
  for(int i = 0; i < 100; i++)
  {
    while(!syncedCams.read(output)){}
    // Pack the two frames into a single mat, and then write the frame
    int rows = output[0].rows;
    int cols = output[0].cols;
    Mat combinedFrame(rows * 2, cols,output[0].type());
    output[0].copyTo(combinedFrame.rowRange(0,rows).colRange(0,cols));
    output[1].copyTo(combinedFrame.rowRange(rows, 2*rows).colRange(0,cols));
    writer.write(combinedFrame);
  }
}

int main(int argc, char** argv )
{
  //runThreshold(1);
  runSyncTest();
  return 0;
}
