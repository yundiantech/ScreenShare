#ifndef VIDEOFRAME_H
#define VIDEOFRAME_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <memory>

#include "NALU/nalu.h"

#define VideoFramePtr std::shared_ptr<VideoFrame>

class VideoFrame
{
public:
    VideoFrame();
    ~VideoFrame();

    void setNalu(uint8_t *buffer, const int &len, const T_NALU_TYPE &type);

    T_NALU *getNalu(){return mNalu;}

private:
    T_NALU *mNalu;

};

#endif // VIDEOFRAME_H
