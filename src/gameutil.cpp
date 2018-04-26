#include "gameutil.hpp"
#include <iostream>
#include <string>
#include <sstream>

using namespace cv;
using namespace std;

#define SSTR( x ) static_cast< std::ostringstream & >( ( std::ostringstream() << std::dec << x ) ).str()

#define RETRY_NUM 5
#define SCORE_POS Point(30,30)
#define TEXT_COLOR Scalar(200,200,250)
#define TEXT_SIZE 1

int init_camera(VideoCapture* cap, int hres, int vres)
{
  int i;
  for(i = 0; i < RETRY_NUM; ++i)
  {
    cap->open(i);

    if(!cap->isOpened())
    {
      cout << "Failed to open camera " << i << endl;
    }
    else
    {
      break;
    }
  }

  if(!cap->isOpened())
  {
    cout << "Failed to open video stream" << endl;
    return -1;
  }

  cap->set(CV_CAP_PROP_FRAME_WIDTH,hres);
  cap->set(CV_CAP_PROP_FRAME_HEIGHT,vres);

  return 1;
}


int write_ui(Mat image, int score)
{
  std::string scoreString = "Score: " + SSTR(score);

  putText(image, scoreString, SCORE_POS, FONT_HERSHEY_COMPLEX_SMALL, TEXT_SIZE, TEXT_COLOR, 1, CV_AA);
  
}