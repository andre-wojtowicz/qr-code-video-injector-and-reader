#include <iostream>
#include "include/yuv422_loader.h"

using namespace std;
YUV422_Loader::YUV422_Loader(int width, int height)
{
    this->width  = width;
    this->height = height;
}

cv::Mat* YUV422_Loader::grab_y_frame(char* frame_raw)
{
    cv::Mat mat_frame_raw(1, (this->width * 2) * this->height, CV_8UC1, frame_raw);
    cv::Mat channels[4];

    mat_frame_raw = mat_frame_raw.reshape(4);
    cv::split(mat_frame_raw, channels);

    cv::Mat& Y1 = channels[1];
    cv::Mat& Y2 = channels[3];

    cv::Mat YY_arr[2] = { Y1, Y2 };
    cv::Mat YY;

    cv::merge(YY_arr, 2, YY);

    YY = YY.reshape(1, this->height);

    cv::Mat* mat_frame_y = new cv::Mat(YY);

    return mat_frame_y;
}
