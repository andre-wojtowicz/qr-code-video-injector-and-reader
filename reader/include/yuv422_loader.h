#ifndef Y422_loader_H
#define Y422_loader_H

#include <opencv2/core/core.hpp>
#include <iostream>
class YUV422_Loader
{
public:
    YUV422_Loader(int width, int height);

    cv::Mat* grab_y_frame(char* frame_raw);

private:
    int width;
    int height;
};

#endif
