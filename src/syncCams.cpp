#include <cassert>

#include "opencv2/opencv.hpp"
#include "syncCams.h"


using namespace cv;

SyncCams::SyncCams (vector<int>camIndexes)
{
  for(int i = 0; i < camIndexes.size(); i++)
  {
    caps.push_back(VideoCapture(camIndexes[i]));
  }
}

vector<double> SyncCams::get(int propId)
{
  vector<double> result;
  for(int i = 0; i < caps.size(); i++)
  {
    result.push_back(caps[i].get(propId));
  }
  return result;
}

bool SyncCams::isOpened()
{
  bool opened = true;
  for(int i = 0; i < caps.size(); i++)
  {
    opened &= caps[i].isOpened();
  }
  return opened;
}

bool SyncCams::read(OutputArrayOfArrays out)
{
  return true;
}

vector<bool> SyncCams::set(int propId, double value)
{
  vector<bool> result;
  for(int i = 0; i < caps.size(); i++)
  {
    result.push_back(caps[i].set(propId,value));
  }
  return result;
}

void *cameraWorker(void *arg)
{
  
}
