#include "MediaReader.h"

#include "AppConfig.h"
#include "RtpSender/RtpSenderManager.h"

MediaReader::MediaReader()
{
    mGetVideoThread = nullptr;
    mGetAudioThread = nullptr;
}

ErroCode MediaReader::init()
{
    mGetVideoThread = new GetVideoThread();
    mGetAudioThread = new GetAudioThread();

    ErroCode code;

    if (!mGetVideoThread->init())
    {
        code = VideoOpenFailed;
    }
    else if (!mGetAudioThread->init())
    {
        code = AudioOpenFailed;
    }
    else
    {
        code = SUCCEED;
    }

    return code;
}

void MediaReader::setRemoteIp(char *ip, const int &port)
{
    AppConfig::gRtpSenderManager->setRemoteIp(ip, port);
}

void MediaReader::setQuantity(int value)
{
    if (mGetVideoThread != nullptr)
    {
        mGetVideoThread->setQuantity(value);
    }
}

bool MediaReader::start()
{
    if (mGetVideoThread != nullptr)
    {
        mGetVideoThread->startRecord();
    }

    if (mGetAudioThread != nullptr)
    {
        mGetAudioThread->startRecord();
    }

    AppConfig::gRtpSenderManager->startSend();

    return true;
}

bool MediaReader::stop()
{
    if (mGetVideoThread != nullptr)
    {
        mGetVideoThread->stopRecord();
    }

    if (mGetAudioThread != nullptr)
    {
        mGetAudioThread->stopRecord();
    }

    AppConfig::gRtpSenderManager->stopSend();

    return true;
}

