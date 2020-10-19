/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#include <stdio.h>
#include <string.h>

#if defined(WIN32)
    #include <WinSock2.h>
    #include <Windows.h>
#else
    #include <pthread.h>
    #include <errno.h>
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>

    #define closesocket close

#endif

#include "AppConfig.h"
#include "RtpSender.h"

const int MTU_SIZE = 1300;

#define RTP_HEADER_SIZE 12
#define H264        96
#define AAC         97

//
// This function checks if there was a RTP error. If so, it displays an error
// message and exists.
//
void checkerror(int rtperr)
{
    if (rtperr < 0)
    {
//        std::cout << "ERROR: " << RTPGetErrorString(rtperr) << std::endl;
//        exit(-1);
    }
}

RtpSender::RtpSender(char *ip, int port)
{
    initRtp(ip, port);
}

bool RtpSender::initRtp(char *ip, int port)
{
#ifdef WIN32
    WSADATA dat;
    WSAStartup(MAKEWORD(2,2),&dat);
#endif // WIN32

#if USE_JRTPLIB

    int status;
    uint16_t portbase = 7000; //本地端口

    uint16_t destport = port;    //目标端口
    uint32_t destip = inet_addr(ip); //目标IP

    if (destip == INADDR_NONE)
    {
//        std::cerr << "Bad IP address specified" << std::endl;
        return false;
    }
    destip = ntohl(destip);

    // Now, we'll create a RTP session, set the destination, send some
    // packets and poll for incoming data.

    RTPUDPv4TransmissionParams transparams;
    RTPSessionParams sessparams;

    // IMPORTANT: The local timestamp unit MUST be set, otherwise
    //            RTCP Sender Report info will be calculated wrong
    // In this case, we'll be sending 10 samples each second, so we'll
    // put the timestamp unit to (1.0/10.0)
//    sessparams.SetOwnTimestampUnit(1.0/10.0);

    sessparams.SetOwnTimestampUnit(1.0/90000.0);

    sessparams.SetAcceptOwnPackets(true);

    do
    {
        transparams.SetPortbase(portbase++);  //设置本地端口
        status = sess.Create(sessparams,&transparams);

    }while(status<0);

    checkerror(status);

    RTPIPv4Address addr(destip, destport);
    status = sess.AddDestination(addr);  //设置目标IP和端口
    checkerror(status);

//    sess.SetDefaultPayloadType(H264);
//    sess.SetDefaultMark(false);
//    sess.SetDefaultTimestampIncrement(100);

#else

    strcpy(DEST_IP, ip);
    DEST_PORT = port;

    struct sockaddr_in server;
    int len =sizeof(server);

    server.sin_family=AF_INET;
    server.sin_port=htons(DEST_PORT);
    server.sin_addr.s_addr=inet_addr(DEST_IP);
    socket1=socket(AF_INET,SOCK_DGRAM,0);
    connect(socket1, (const sockaddr *)&server, len);//申请UDP套接字

#endif

    return  true;
}

void RtpSender::sendAnNalu(T_NALU *n, uint32_t pts)
{
    if (n->type == T_NALU_H264)
    {
        sendH264Nalu(&n->nalu.h264Nalu, pts);
    }
    else
    {
        sendH265Nalu(&n->nalu.h265Nalu, pts);
    }
}

void RtpSender::sendAAC(uint8_t * buffer, int len, uint32_t pts)
{

//fprintf(stderr, "%s size=%d pts=%d\n", __FUNCTION__, len, pts);

///打包AAC的还有问题，暂未实现。

    uint8_t sendbuf[1500] = {0};

    int aacBufLen = len;

    sendbuf[0] = 0x00;
    sendbuf[1] = 0x10;

    sendbuf[2] = (len & 0x1fe0) >> 5;
    sendbuf[3] = (len & 0x1f) << 3;

    memcpy(&sendbuf[4], buffer, aacBufLen);

    int sendLen = len + 4;

//    memcpy(sendbuf, buffer, aacBufLen);
//    sendLen = len;

//        if (aacBufLen < POCKT_MAX_LEN)
//        {
//            finalSendLen = aacBufLen + RTP_HEADER_SIZE + 1;
//            sendbuf[RTP_HEADER_SIZE] = aacBufLen;
//            memcpy(sendbuf + RTP_HEADER_SIZE + 1, frame, aacBufLen);
//        }
//        else if (aacBufLen >= POCKT_MAX_LEN)
//        {
//            finalSendLen = aacBufLen + RTP_HEADER_SIZE + 2;
//            sendbuf[RTP_HEADER_SIZE] = POCKT_MAX_LEN ;
//            sendbuf[RTP_HEADER_SIZE+1] = aacBufLen % POCKT_MAX_LEN;
//            memcpy(sendbuf + RTP_HEADER_SIZE + 2, frame, aacBufLen);
//        }

    int marker = 1;

    sendRtpPacket(sendbuf, sendLen, AAC, pts, seq_num++, marker); //打包rtp发送数据

}

void RtpSender::sendRtpPacket(uint8_t * buffer, const int &len, const uint8_t &payloadType, const uint32_t &pts, const uint16_t &seqNum, const bool &mark)
{

    uint32_t incrementedTime = pts - ts_current;
    ts_current = pts;

#if USE_JRTPLIB
    ///jrtplib只需要传入跟上一帧的时间戳差值
    int ret = sess.SendPacket(buffer, len, payloadType, mark, incrementedTime);
//    fprintf(stderr, "%d %d %d %d \n", len, ret, pts, incrementedTime);
#else
    ///1.先生成rtp头部数据（12字节）

    char sendbuf[1600] = {0};

    RTP_HEADER *rtp_hdr  = (RTP_HEADER*)&sendbuf[0];
    //设置RTP HEADER，
    rtp_hdr->payload     = payloadType;  //负载类型号，
    rtp_hdr->version     = 2;  //版本号，此版本固定为2
    rtp_hdr->marker      = 0;   //标志位，由具体协议规定其值。
    rtp_hdr->ssrc        = htonl(10);    //随机指定为10，并且在本RTP会话中全局唯一
    //设置rtp M 位；
    rtp_hdr->marker = mark;
    rtp_hdr->seq_no = htons(seqNum); //序列号
    rtp_hdr->timestamp = htonl(ts_current);

    ///2.将要发送的数据(buffer) 加到rtp头后面
    char *nalu_payload = &sendbuf[12];
    memcpy(nalu_payload, buffer, len);

    int bytes = len + 12;	//加上rtp_header的固定长度12字节
    send(socket1, sendbuf, bytes, 0);//发送rtp包

#endif

}

void RtpSender::sendH264Nalu(T_H264_NALU *n, const uint32_t &pts)
{

    bool marker  = 0;   //标志位，由具体协议规定其值。

    if(n->len <= MTU_SIZE)
    {
        ///当一个NALU小于1400字节的时候，采用一个单RTP包发送

        //设置rtp M 位；
        marker = 1;

        /*
         * 发送单个NAL单元，需要先剔除NAL单元头部，然后再加上如下的一个字节数据再发送。
         * 然而可以发现，这个结构和NALU头部是一样的。
         * 因此说白了，发送单个NAL单元，只需要加上rtp头直接发送就行
         * +---------------+
            |0|1|2|3|4|5|6|7|
            +-+-+-+-+-+-+-+-+
            |F|NRI|  Type   |
            +---------------+
         */

        uint8_t buffer[1500] = {0};

        uint8_t* nalu_payload = &buffer[0];//同理将sendbuf[0]赋给nalu_payload
        memcpy(nalu_payload, n->buf, n->len);//去掉nalu头的nalu剩余内容写入sendbuf[1]开始的字符串。

        int size = n->len;

        sendRtpPacket(buffer, size, H264, pts, seq_num++, marker); //打包rtp发送数据

    }
    else
    {
        ///数据超过1400字节，则安装FU-A的RTP荷载格式来发送
        ///FU-A的RTP荷载格式如下
        /*
         * 先加上如下一个字节的分片单元指示数据
         * +---------------+
            |0|1|2|3|4|5|6|7|
            +-+-+-+-+-+-+-+-+
            |F|NRI|  Type   |
            +---------------+
         *
         *然后再加上1字节的分片单元头
        +---------------+
        |0|1|2|3|4|5|6|7|
        +-+-+-+-+-+-+-+-+
        |S|E|R|  Type   |
        +---------------+
        */

        //得到该nalu需要用多少长度为1400字节的RTP包来发送
        int packetNums = n->len / MTU_SIZE;//需要k个1400字节的RTP包
        int lastPacketSize = n->len % MTU_SIZE;//最后一个RTP包的需要装载的字节数
        if (lastPacketSize == 0)
        {
            lastPacketSize = MTU_SIZE;
        }
        else
        {
            packetNums++;
        }

        int currentPacketIndex = 0; //当前发送的包序号

        while(currentPacketIndex < packetNums)
        {
            if(currentPacketIndex == 0)//发送一个需要分片的NALU的第一个分片，置FU HEADER的S位
            {
                //设置rtp M 位；
                marker = 0;

                uint8_t buffer[1500] = {0};

                //设置FU INDICATOR,并将这个HEADER填入 buffer[0]
                FU_INDICATOR* fu_ind =(FU_INDICATOR*)&buffer[0]; //将 buffer[0]的地址赋给fu_ind，之后对fu_ind的写入就将写入buffer中；
                fu_ind->F    = n->forbidden_bit;
                fu_ind->NRI  = n->nal_reference_idc>>5;
                fu_ind->TYPE = 28;

                //设置FU HEADER,并将这个HEADER填入buffer[1]
                FU_HEADER* fu_hdr = (FU_HEADER*)&buffer[1];
                fu_hdr->E    = 0;
                fu_hdr->R    = 0;
                fu_hdr->S    = 1; //S: 1 bit 当设置成1,开始位指示分片NAL单元的开始。当跟随的FU荷载不是分片NAL单元荷载的开始，开始位设为0。
                fu_hdr->TYPE = n->nal_unit_type;

                uint8_t* nalu_payload = &buffer[2];//同理将buffer[2]赋给nalu_payload
                memcpy(nalu_payload, n->buf+1, MTU_SIZE-1 );//去掉NALU头

                int size = MTU_SIZE-1 + 2; //长度为nalu的长度（除去起始前缀和NALU头）加上fu_ind，fu_hdr的固定长度2字节

                sendRtpPacket(buffer, size, H264, pts, seq_num++, marker); //打包rtp发送数据

                AppConfig::mSleep(2); //休眠2Ms 不能发太快了
            }
            //发送一个需要分片的NALU的非第一个分片，清零FU HEADER的S位，如果该分片是该NALU的最后一个分片，置FU HEADER的E位
            else if(currentPacketIndex==(packetNums-1))//发送的是最后一个分片，注意最后一个分片的长度可能超过1400字节（当l>1386时）。
            {

                //设置rtp M 位；当前传输的是最后一个分片时该位置1
                marker = 1;

                uint8_t buffer[1500] = {0};

                //设置FU INDICATOR,并将这个HEADER填入 buffer[0]
                FU_INDICATOR* fu_ind =(FU_INDICATOR*)&buffer[0]; //将 buffer[0]的地址赋给fu_ind，之后对fu_ind的写入就将写入buffer中；
                fu_ind->F    = n->forbidden_bit;
                fu_ind->NRI  = n->nal_reference_idc>>5;
                fu_ind->TYPE = 28;

                //设置FU HEADER,并将这个HEADER填入sendbuf[13]
                FU_HEADER* fu_hdr = (FU_HEADER*)&buffer[1];
                fu_hdr->E    = 1; //E: 1 bit 当设置成1, 结束位指示分片NAL单元的结束，即, 荷载的最后字节也是分片NAL单元的最后一个字节。当跟随的 FU荷载不是分片NAL单元的最后分片,结束位设置为0。
                fu_hdr->R    = 0;
                fu_hdr->S    = 0;
                fu_hdr->TYPE = n->nal_unit_type;

                uint8_t* nalu_payload = &buffer[2];//同理将buffer[2]赋给nalu_payload
                memcpy(nalu_payload, n->buf+currentPacketIndex*MTU_SIZE, lastPacketSize);

                int size = lastPacketSize + 2; //加上fu_ind，fu_hdr的固定长度2字节

                sendRtpPacket(buffer, size, H264, pts, seq_num++, marker); //打包rtp发送数据

            }
            else
            {
                //设置rtp M 位；
                marker = 0;


                uint8_t buffer[1500] = {0};

                //设置FU INDICATOR,并将这个HEADER填入 buffer[0]
                FU_INDICATOR* fu_ind =(FU_INDICATOR*)&buffer[0]; //将 buffer[0]的地址赋给fu_ind，之后对fu_ind的写入就将写入buffer中；
                fu_ind->F    = n->forbidden_bit;
                fu_ind->NRI  = n->nal_reference_idc>>5;
                fu_ind->TYPE = 28;

                //设置FU HEADER,并将这个HEADER填入sendbuf[13]
                FU_HEADER* fu_hdr = (FU_HEADER*)&buffer[1];
                fu_hdr->E    = 0; //E: 1 bit  FU荷载不是分片NAL单元的最后分片,结束位设置为0。
                fu_hdr->R    = 0;
                fu_hdr->S    = 0;
                fu_hdr->TYPE = n->nal_unit_type;

                uint8_t* nalu_payload = &buffer[2];//同理将buffer[2]赋给nalu_payload
                memcpy(nalu_payload, n->buf+currentPacketIndex*MTU_SIZE, MTU_SIZE);

                int size = MTU_SIZE + 2; //加上fu_ind，fu_hdr的固定长度2字节

                sendRtpPacket(buffer, size, H264, pts, seq_num++, marker); //打包rtp发送数据

                AppConfig::mSleep(2); //休眠2Ms 不能发太快了
            }

            currentPacketIndex++;
        }
    }

//    printf("send finished\n");
}

void RtpSender::sendH265Nalu(T_H265_NALU *n, const uint32_t &pts)
{
    ///h265打包rtp目前暂未实现。
    fprintf(stderr, "erro while send h265 ...\n");

}
