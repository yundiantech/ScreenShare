#ifndef RTPRECEIVER_H
#define RTPRECEIVER_H

#include "rtp.h"
#include "VideoDecoder/VideoDecoder.h"

#include "Video/VideoEventHandle.h"

#define USE_JRTPLIB 1

#if USE_JRTPLIB
    #include "rtpsession.h"
    #include "rtpudpv4transmitter.h"
    #include "rtpipv4address.h"
    #include "rtpsessionparams.h"
    #include "rtperrors.h"
    #include "rtppacket.h"
    #include "rtpsourcedata.h"

    using namespace jrtplib;
#else

#endif

typedef struct
{
    unsigned char v;               //!< 版本号
    unsigned char p;			   //!< Padding bit, Padding MUST NOT be used
    unsigned char x;			   //!< Extension, MUST be zero
    unsigned char cc;       	   //!< CSRC count, normally 0 in the absence of RTP mixers
    unsigned char m;			   //!< Marker bit 如果单包 m = 1 ,分片包的最后一个包 = 1，分片包其他包 m = 0 ,组合包不准确
    unsigned char pt;			   //!< 7 bits, 负载类型 H264 96
    unsigned int seq;			   //!< 包号
    unsigned int timestamp;	       //!< timestamp, 27 MHz for H.264 事件戳
    unsigned int ssrc;			   //!< 真号
    unsigned char *payload;      //!< the payload including payload headers
    unsigned int paylen;		   //!< length of payload in bytes
} RTPpacket_t;

class RtpReceiver
{

public:
    RtpReceiver();

    void startReceive(const int &port);
    void stopReceive();

    /**
     * @brief setVideoCallBack 设置视频回调函数
     * @param pointer
     */
    void setVideoCallBack(VideoCallBack *pointer){mVideoCallBack=pointer;}

protected:
    void run();

private:
    bool mIsStop;
    bool mIsThreadRunning;

    uint8_t *mH264Buffer;
    int mH264BufferSize;

    int mPort;
    int mFrameNums;

    VideoDecoder *mH264Decorder;

    /**
     * @brief 解包rtp
     *
     * @param[in] payloadBuffer rtp数据包中payload
     * @param[in] payloadBufferSize 大小
     *
     * @param[out] gotFrame 是否获取到了完整的一帧数据（通过获取最后一个包来判断）
     *
     * @return 非NULL：成功@n
     *         NULL：发生错误
     */
    RTPpacket_t* unpackPayloadBuffer(RTP_HEADER * rtpHeader, uint8_t *payloadBuffer, const int &payloadBufferSize, int *gotFrame);

    ///显示
    void displayPacket(RTPpacket_t *p, const int &gotFrame);


    ///回调函数相关，主要用于输出信息给界面
private:
    ///回调函数
    VideoCallBack *mVideoCallBack;

    ///显示视频数据，此函数不宜做耗时操作，否则会影响播放的流畅性。
    void doDisplayVideo(const uint8_t *yuv420Buffer, const int &width, const int &height, const int &frameNum);


};

#endif // RTPRECEIVER_H
