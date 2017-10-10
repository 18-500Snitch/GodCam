#include <cassert>
#include <stdio.h>
#include "opencv2/opencv.hpp"
#include "syncCams.h"


using namespace cv;

/*
  Init the synced cameras by creating the videoCaptures, the data queues and
  the worker threads.
*/
SyncCams::SyncCams (vector<int>camIndexes)
{
  for(size_t i = 0; i < camIndexes.size(); i++)
  {
    caps.push_back(new VideoCapture(camIndexes[i]));
    matQueue.push_back(new ReaderWriterQueue<FrameTimestamp>(30));
  }
  numCaps = camIndexes.size();
  b = new Barrier(numCaps + 1);
  for(size_t i = 0; i < numCaps; i++)
  {
    workers.push_back(thread(&SyncCams::cameraWorker,this,i));
    workers[i].detach();
  }
}

/*
  Get the desired VideoCapture property of all cameras
*/
vector<double> SyncCams::get(int propId)
{
  vector<double> result;
  for(size_t i = 0; i < numCaps; i++)
  {
    result.push_back(caps[i]->get(propId));
  }
  return result;
}

/*
  Check if all the cameras are opened
*/
bool SyncCams::isOpened()
{
  bool opened = true;
  for(size_t i = 0; i < numCaps; i++)
  {
    opened &= caps[i]->isOpened();
  }
  return opened;
}

/*
  Get the next set of time synced frames
*/
bool SyncCams::read(vector<Mat> &out)
{
  vector<FrameTimestamp> frames(numCaps);
  bool result = true;
  int newestI = 0;
  unsigned long long newestT = 0;
  vector<unsigned long long> timeDiff(numCaps);

  // Check if each queue has a valid frame
  for(size_t i = 0; i < numCaps; i++)
  {
    if(matQueue[i]->peek() == nullptr)
    {
      result &= false;
    }
  }
  if(result)
  {
    // Get the first frame of each queue, while getting the time/index
    // of the most recent frame in time.
    for(size_t i = 0; i < numCaps; i++)
    {
      matQueue[i]->try_dequeue(frames[i]);
      newestI = (frames[i].timestamp > newestT) ? i:newestI;
      newestT = frames[newestI].timestamp;
    }
    for(size_t i = 0; i < numCaps; i++)
    {
      timeDiff[i] = newestT - frames[i].timestamp;
      cout << timeDiff[i] << " ";
    }
    cout << "\n";

    // for(size_t i = 0; i < numCaps; i++)
    // {
    //   while(timeDiff[i] >= max_frame_diff)
    //   {
    //     matQueue[i]->try_dequeue(frames[i]);
    //     timeDiff[i] = newestT - frames[i].timestamp;
    //   }
    // }
    for(size_t i = 0; i < numCaps; i++)
    {
      out[i] = frames[i].frame.clone();
    }
  }
  return result;
}

/*
  Set the desired VideoCpature property
*/
vector<bool> SyncCams::set(int propId, double value)
{
  vector<bool> result;
  for(size_t i = 0; i < numCaps; i++)
  {
    result.push_back(caps[i]->set(propId,value));
  }
  return result;
}

/*
  Notify each thread to start capturing frames from the camera
*/
bool SyncCams::startCapturing()
{
  if(!isOpened())
  {
    return false;
  }
  // Get the fps of the cameras to set the right frame limit interval
  vector<double> fps = get(CAP_PROP_FPS);
  max_frame_diff = 1000/((unsigned long long)fps[0]);
  b->Wait();
  return true;
}

/*
  Worker thread that captures each camera frame, timestamps it, and writes it
  to the queue
*/
void SyncCams::cameraWorker(int index)
{
  cout << index << "\n";

  // Get the video camera to read from
  VideoCapture cap = *(this->caps[index]);

  // Get the queue to write to
  ReaderWriterQueue<FrameTimestamp> *queue = matQueue[index];

  // Wait for each thread to be ready to capture frames
  b->Wait();

  int ex = static_cast<int>(cap.get(CAP_PROP_FOURCC));
  char EXT1[] = {
    (char)(ex & 0XFF),
    (char)((ex & 0XFF00) >> 8),
    (char)((ex & 0XFF0000) >> 16),
    (char)((ex & 0XFF000000) >> 24),
    0
  };
  printf("Cam FPS: %llu  Res: %f x %f Format: %s\n",
  (unsigned long long)cap.get(CAP_PROP_FPS),
  cap.get(CAP_PROP_FRAME_WIDTH),
  cap.get(CAP_PROP_FRAME_HEIGHT),
  EXT1);

  // Grab, decode, and send the next frame to the camera.
  while(1)
  {
    if(cap.grab())
    {
      unsigned long long frame_time = cap.get(CAP_PROP_POS_MSEC);
      Mat frame;
      if(cap.retrieve(frame))
      {
        FrameTimestamp data(frame,frame_time);
        queue->try_enqueue(data);
      }
    }
  }
}
