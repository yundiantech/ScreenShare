/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#ifndef GetVideoThread_H
#define GetVideoThread_H

extern "C"
{
    #include "libavcodec/avcodec.h"
    #include "libavformat/avformat.h"
    #include "libswscale/swscale.h"
    #include "libavdevice/avdevice.h"

    #include "libavutil/imgutils.h"
}

#include "VideoEncoder.h"

/**
 * @brief The GetVideoThread class  此类主要负责采集屏幕
 */

class GetVideoThread
{

public:
    explicit GetVideoThread();
    ~GetVideoThread();

    bool init();
    void deInit();

    void setQuantity(int value);

    void startRecord();
    void pauseRecord();
    void restoreRecord();
    void stopRecord();

protected:
    void run();

private:

    AVFormatContext	*pFormatCtx;
    int             videoStream;
    AVCodecContext	*pCodecCtx;

    AVFrame	*pFrame,*pFrameYUV;
    uint8_t *outBufferYuv;

    bool m_isRun;
    bool m_pause;

    bool m_getFirst; //是否获取到了时间基准

    VideoEncoder *mVideoEncoder;

};

#endif // GetVideoThread_H
