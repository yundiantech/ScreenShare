#include "RtpSenderManager.h"

#include <stdlib.h>
#include <string.h>

RtpSenderManager::RtpSenderManager()
{
    mIsStop = false;
    mRtpSender =  nullptr;
    mCond = new Cond();
}

RtpSenderManager::~RtpSenderManager()
{

}

void RtpSenderManager::setRemoteIp(char *ip, int port)
{
    mRtpSender = new RtpSender(ip, port);
}

void RtpSenderManager::sendVideo(VideoFramePtr framePtr, const uint32_t &pts)
{
    T_FRAME frame;
    frame.type = T_FRAME_VIDEO;
    frame.pts  = pts * 90; //换算成相对于90000的时间戳 pts / 1000.0 * 90000;
    frame.videoFrame = framePtr;

    inputFrame(frame);
}

void RtpSenderManager::sendAudio(AACFramePtr framePtr, const uint32_t &pts)
{
    T_FRAME frame;
    frame.type = T_FRAME_AUDIO;
    frame.pts  = pts * 44.1; //换算成相对于44100的时间戳 pts / 1000.0 * 44100;
    frame.audioFrame = framePtr;

    inputFrame(frame);
}

void RtpSenderManager::inputFrame(const T_FRAME &frame)
{
    mCond->Lock();
    mFrameList.push_back(frame);
    mCond->Unlock();
    mCond->Signal();
}

void RtpSenderManager::startSend()
{
    mIsStop = false;

    //启动新的线程
    std::thread([&](RtpSenderManager *pointer)
    {
        pointer->run();

    }, this).detach();
}

void RtpSenderManager::stopSend()
{
    mIsStop = true;
    mCond->Signal();
}

void RtpSenderManager::run()
{
//    int frameNum = 0; //当前播放的帧序号

    while(!mIsStop)
    {
        mCond->Lock();

        while (mFrameList.empty())
        {
            if (mIsStop) break;

            mCond->Wait();
        }

        if (mIsStop)
        {
            mCond->Unlock();
            break;
        }

        T_FRAME frame = mFrameList.front();
        mFrameList.pop_front();

        mCond->Unlock();

        if (frame.type == T_FRAME_VIDEO)
        {
//        /// 重新解码显示到界面上，这里我们编码又解码，属于无用功，浪费cpu，
//        /// 只是demo就将就着这样了，有兴趣的可以改成由编码线程直接传递rgb或者yuv数据过来。
//        {
//            uint8_t *bufferRGB;
//            int width;
//            int height;

//            ///这读取到的nalu已经剔除起始码了
//            ///而传给ffmpeg解码的数据需要带上起始码
//            ///因此需要给他加一个上去
//            int h264BufferSize = nalu->len + nalu->startcodeprefix_len;
//            unsigned char * h264Buffer = (unsigned char *)malloc(h264BufferSize);

//            if (nalu->startcodeprefix_len == 4)
//            {
//                h264Buffer[0] = 0x00;
//                h264Buffer[1] = 0x00;
//                h264Buffer[2] = 0x00;
//                h264Buffer[3] = 0x01;
//            }
//            else
//            {
//                h264Buffer[0] = 0x00;
//                h264Buffer[1] = 0x00;
//                h264Buffer[2] = 0x01;
//            }

//            memcpy(h264Buffer+nalu->startcodeprefix_len, nalu->buf, nalu->len);
//            int ret = mH264Decorder->decodeH264(h264Buffer, h264BufferSize, bufferRGB, width, height);
//            free(h264Buffer);

//            int frameRate = mH264Decorder->getFrameRate(); //获取帧率

//            //把这个RGB数据 放入QIMage
//            QImage image = QImage((uchar *)bufferRGB, width, height, QImage::Format_RGB32);

//            //然后传给主线程显示
//            emit sig_GetOneFrame(image.copy());
//        }

            if (mRtpSender != NULL)
                mRtpSender->sendAnNalu(frame.videoFrame->getNalu(), frame.pts); //发送rtp
        }
        else
        {
            frame.audioFrame->getBuffer();
            frame.audioFrame->getSize();
            if (mRtpSender != NULL)
                mRtpSender->sendAAC(frame.audioFrame->getBuffer(), frame.audioFrame->getSize(), frame.pts); //发送rtp
        }
    }

}
