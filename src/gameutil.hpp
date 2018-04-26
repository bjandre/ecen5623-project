#ifndef GAME_UTIL_HPP
#define GAME_UTIL_HPP

#include <opencv2/opencv.hpp>

int init_camera(VideoCapture* cap, int hres, int vres);

int write_ui(Mat image, int score);

#endif