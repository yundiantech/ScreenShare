/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#ifndef RTPSENDER_H
#define RTPSENDER_H

#include "rtp.h"
#include "NALU/nalu.h"

#define USE_JRTPLIB 1

#if USE_JRTPLIB
    #include "rtpsession.h"
    #include "rtpudpv4transmitter.h"
    #include "rtpipv4address.h"
    #include "rtpsessionparams.h"
    #include "rtperrors.h"
    #include "rtppacket.h"

    using namespace jrtplib;
#else

#endif

class RtpSender
{
public:
    RtpSender(char *ip, int port);

    void sendAnNalu(T_NALU *n, uint32_t pts); //发送一个NALU
    void sendAAC(uint8_t * buffer, int len, uint32_t pts);

private:
#if USE_JRTPLIB
    /// JRTPLIB 相关的对象
    RTPSession sess;
    uint16_t m_hdrextID;
    void * m_hdrextdata;
    size_t m_numhdrextwords;
#else
    int    socket1;

    char DEST_IP[36];
    int DEST_PORT;
#endif

    unsigned int ts_current=0;
    unsigned short seq_num =0;

    bool initRtp(char *ip, int port);

    /**
     * @brief sendRtpPacket  将数据打包成rtp包发送
     * @param buffer  [in] 要发送的数据
     * @param len     [in] 数据长度
     * @param pts     [in] 时间戳
     * @param seqNum  [in] rtp包序号
     * @param mark    [in] mark标志位
     */
    void sendRtpPacket(uint8_t * buffer, const int &len, const uint8_t &payloadType, const uint32_t &pts, const uint16_t &seqNum, const bool &mark);

    void sendH264Nalu(T_H264_NALU *n, const uint32_t &pts);
    void sendH265Nalu(T_H265_NALU *n, const uint32_t &pts);

};

#endif // RTPSENDER_H
