#ifndef SYNCCAMS
#define SYNCCAMS


#include <vector>


#include "opencv2/opencv.hpp"
#include "pthread.h"
#include "readerwriterqueue.h"

using namespace cv;
using namespace std;
using namespace moodycamel;

// Struct to hold the frame mat and the timestamp.
typedef struct frameTimestamp_t
{
  Mat frame;
  double timestamp;
  frameTimestamp_t(const frameTimestamp_t& f) :
    frame((f.frame).clone()), timestamp(f.timestamp){}
} FrameTimestamp;

class SyncCams
{
  public:
    SyncCams(vector<int>camIndexes);
    vector<double> get(int propId);
    bool isOpened();
    bool read(OutputArrayOfArrays out);
    vector<bool> set(int propId, double value);

  private:
    vector<VideoCapture> caps;
    vector<pthread_t> threads;
    vector<ReaderWriterQueue<FrameTimestamp> > matQueue;
    void *cameraWorker(void *arg);
};

#endif
