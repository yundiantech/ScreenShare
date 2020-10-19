#ifndef MEDIAREADER_H
#define MEDIAREADER_H

#include "Video/GetVideoThread.h"
#include "Audio/GetAudioThread.h"

enum ErroCode
{
    AudioOpenFailed = 0,
    VideoOpenFailed,
    SUCCEED
};

class MediaReader
{
public:
    MediaReader();

    ErroCode init();

    void setRemoteIp(char *ip, const int &port);
    void setQuantity(int value);

    bool start();
    bool stop();

private:
    GetVideoThread *mGetVideoThread;
    GetAudioThread *mGetAudioThread;
};

#endif // MEDIAREADER_H
