#ifndef SYNCCAMS
#define SYNCCAMS


#include <vector>
#include <thread>


#include "opencv2/opencv.hpp"
#include "readerwriterqueue.h"
#include "barrier.h"

using namespace cv;
using namespace std;
using namespace moodycamel;

// Struct to hold the frame mat and the timestamp.
typedef struct frameTimestamp_t
{
  Mat frame;
  unsigned long long timestamp;
  frameTimestamp_t(const frameTimestamp_t& f) :
    frame((f.frame).clone()), timestamp(f.timestamp){}
  frameTimestamp_t(Mat m, unsigned long long t) :
    frame(m), timestamp(t){}
  frameTimestamp_t(){}
} FrameTimestamp;

class SyncCams
{
  public:
    SyncCams(vector<int>camIndexes);
    vector<double> get(int propId);
    bool isOpened();
    bool read(vector<Mat> &out);
    vector<bool> set(int propId, double value);
    bool startCapturing();

  private:
    vector<VideoCapture*> caps;
    size_t numCaps;
    vector<thread> workers;
    vector<ReaderWriterQueue<FrameTimestamp>* > matQueue;
    unsigned long long max_frame_diff;
    Barrier *b;
    void cameraWorker(int index);
};

#endif
