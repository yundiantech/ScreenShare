#ifndef RTPSENDERMANAGER_H
#define RTPSENDERMANAGER_H

#include <list>
#include <thread>

#include "Mutex/Cond.h"
#include "RtpSender.h"

#include "Media/Audio/AudioFrame/AACFrame.h"
#include "Media/Video/VideoFrame/VideoFrame.h"

enum T_FRAME_TYPE
{
    T_FRAME_VIDEO = 0,
    T_FRAME_AUDIO,
};

struct T_FRAME
{
    T_FRAME_TYPE type;

    uint32_t pts;

    VideoFramePtr videoFrame;
    AACFramePtr   audioFrame;

    T_FRAME()
    {
        pts = 0;
        videoFrame = nullptr;
        audioFrame = nullptr;
    }

};

class RtpSenderManager
{
public:
    explicit RtpSenderManager();
    ~RtpSenderManager();

    void startSend();
    void stopSend();

    void setRemoteIp(char *ip, int port);

    void sendVideo(VideoFramePtr framePtr, const uint32_t &pts);
    void sendAudio(AACFramePtr framePtr, const uint32_t &pts);

protected:
    void run();

private:
    bool mIsStop;

    RtpSender *mRtpSender; //打包发送rtp数据的类

    Cond *mCond;
    std::list<T_FRAME> mFrameList;

    void inputFrame(const T_FRAME &frame);

};

#endif // RTPSENDERMANAGER_H
