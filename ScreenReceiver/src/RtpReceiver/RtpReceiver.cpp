#include "RtpReceiver.h"

#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <thread>
#include <iostream>

#if defined(WIN32)
    #include <WinSock2.h>
    #include <Windows.h>
#else
    #include <pthread.h>
    #include <time.h>
#endif

#include "AppConfig.h"

#define  MAXDATASIZE 1500

#define RTP_HEADER_LENTH 12

//负载类型
#define  H264        96
#define  AAC         97

typedef struct
{
  int startcodeprefix_len;      //! 4 for parameter sets and first slice in picture, 3 for everything else (suggested)
  unsigned len;                 //! Length of the NAL unit (Excluding the start code, which does not belong to the NALU)
  unsigned max_size;            //! Nal Unit Buffer size
  int forbidden_bit;            //! should be always FALSE
  int nal_reference_idc;        //! NALU_PRIORITY_xxxx
  int nal_unit_type;            //! NALU_TYPE_xxxx
  unsigned char *buf;                    //! contains the first byte followed by the EBSP
  unsigned short lost_packets;  //! true, if packet loss is detected
} T_H264_NALU;

RtpReceiver::RtpReceiver()
{
#ifdef WIN32
    WSADATA dat;
    WSAStartup(MAKEWORD(2,2),&dat);
#endif // WIN32

    mH264Buffer = (uint8_t*)malloc(1024*1024*10);
    mH264BufferSize = 0;

    mFrameNums = 0;

    mH264Decorder = new VideoDecoder();
    mH264Decorder->openDecoder(AV_CODEC_ID_H264);

    mIsStop = false;
    mIsThreadRunning = false;
}

void RtpReceiver::startReceive(const int &port)
{
    mPort = port;

    mIsStop = false;

    //启动新的线程
    std::thread([=]()
    {
        this->run();

    }).detach();
}

void RtpReceiver::stopReceive()
{
    mIsStop = true;

    while (mIsThreadRunning)
    {
        AppConfig::mSleep(100);
    }
}

#if USE_JRTPLIB

void RtpReceiver::run()
{
    mIsThreadRunning = true;

    uint16_t portbase = mPort; //本地端口

    RTPSession sess;

    RTPUDPv4TransmissionParams transparams;
    RTPSessionParams sessparams;

    sessparams.SetOwnTimestampUnit(1.0 / 9000.0);
    sessparams.SetUsePollThread(true);

    transparams.SetRTPReceiveBuffer(1024 * 1024 *100); //100M

    sessparams.SetAcceptOwnPackets(true);
    transparams.SetPortbase(portbase);  //设置本地端口
    int status = sess.Create(sessparams, &transparams);

    if (status < 0)
    {
        std::cout << "ERROR: " << RTPGetErrorString(status) << std::endl;
        exit(1);
    }

    uint8_t *h264Buffer = (uint8_t*)malloc(1024*1024);
    int h264BufferSize = 0;

    int frameNum = 0;

    //开始接收流包
    while(!mIsStop)
    {
        sess.BeginDataAccess();

        // check incoming packets
        if (sess.GotoFirstSourceWithData())
        {
            do
            {
                RTPPacket *pack;

                while ((pack = sess.GetNextPacket()) != NULL)
                {
                    RTPSourceData *mRTPSourceData = sess.GetCurrentSourceInfo();
                    uint32_t ssrc = mRTPSourceData->GetSSRC();

fprintf(stderr,"%s 111 ssrc=%d \n", __FUNCTION__, ssrc);

                    RTP_HEADER *rtpHeader = (RTP_HEADER*)pack->GetPacketData();

                    uint8_t *rtpBuffer = pack->GetPacketData();
                    int rtpBufferSize = pack->GetPacketLength();

                    uint8_t* payloadBuffer = (uint8_t*)pack->GetPayloadData();
                    int payloadSize = pack->GetPayloadLength();

                    int gotFrame = 0;
                    RTPpacket_t *p = unpackPayloadBuffer(rtpHeader, payloadBuffer, payloadSize, &gotFrame);

                    displayPacket(p, gotFrame);

                    // we don't longer need the packet, so
                    // we'll delete it
                    sess.DeletePacket(pack);
                }

            } while (sess.GotoNextSourceWithData());
        }

        sess.EndDataAccess();

#ifndef RTP_SUPPORT_THREAD
        status = sess.Poll();
        checkerror(status);
#endif // RTP_SUPPORT_THREAD

    }

    sess.BYEDestroy(RTPTime(10, 0), 0, 0);

    mIsThreadRunning = false;

//    #ifdef WIN32
//        WSACleanup();
//    #endif // WIN32

}

#else

void RtpReceiver::run()
{

    mIsThreadRunning = true;

    int frameNum = 0;

    SOCKET sockServer = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockServer == INVALID_SOCKET)
    {
        fprintf(stderr, "Failed socket() \n");
        return;
    }

    SOCKADDR_IN addr_Server; //服务器的地址等信息
    addr_Server.sin_family = AF_INET;
    addr_Server.sin_port = htons(mPort);
    addr_Server.sin_addr.S_un.S_addr = INADDR_ANY;
    if (bind(sockServer, (SOCKADDR*)&addr_Server, sizeof(addr_Server)) == SOCKET_ERROR)
    {
        //服务器与本地地址绑定
        fprintf(stderr, "Failed socket() %d \n", WSAGetLastError());
        return;
    }

    SOCKADDR_IN addr_Clt;

    fprintf(stderr,"UPD servese is bind success and do_echo now\n");

    uint8_t *h264Buffer = (uint8_t*)malloc(1024*1024);
    int h264BufferSize = 0;

    int fromlen = sizeof(SOCKADDR);

    while(!mIsStop)
    {
        char rtpBuffer[10240] = {0};
        int rtpSize = recvfrom(sockServer, rtpBuffer, 10240, 0, (SOCKADDR*)&addr_Clt, &fromlen);

//fprintf(stderr,"receive size:%d \n", rtpSize);
        RTP_HEADER* rtpHeader =(RTP_HEADER*)&rtpBuffer[0];

        uint8_t* payloadBuffer = (uint8_t*)rtpBuffer + RTP_HEADER_LENTH;
        int payloadSize = rtpSize - RTP_HEADER_LENTH;

        int gotFrame = 0;
        RTPpacket_t *p = unpackPayloadBuffer(rtpHeader, payloadBuffer, payloadSize, &gotFrame);
        displayPacket(p, gotFrame);

    }

    mIsThreadRunning = false;
}

#endif

void RtpReceiver::displayPacket(RTPpacket_t *p, const int &gotFrame)
{
    if (p != nullptr)
    {
        if (p->pt == H264)
        {
            if (mH264BufferSize == 0)
            {
                mH264Buffer[mH264BufferSize++] = 0X00;
                mH264Buffer[mH264BufferSize++] = 0X00;
                mH264Buffer[mH264BufferSize++] = 0X00;
                mH264Buffer[mH264BufferSize++] = 0X01;
            }

            memcpy(mH264Buffer+mH264BufferSize, p->payload, p->paylen);
            mH264BufferSize += p->paylen;

            ///收到了完整的一帧数据
            if (gotFrame)
            {
//                   fprintf(stderr, "got one frame size=%d p->pt=%d\n",mH264BufferSize, p->pt);

               uint8_t *yuv420Buffer = nullptr;
               int width = 0;
               int height = 0;

               int ret = mH264Decorder->decode(mH264Buffer, mH264BufferSize, yuv420Buffer, width, height);
               mH264BufferSize = 0;

               if (yuv420Buffer != nullptr)
               {
                   doDisplayVideo(yuv420Buffer, width, height, mFrameNums++);
               }
            }
        }

        free(p->payload);
        free(p);
    }
}

RTPpacket_t *RtpReceiver::unpackPayloadBuffer(RTP_HEADER * rtpHeader, uint8_t *payloadBuffer, const int &payloadBufferSize, int *gotFrame)
{
    RTPpacket_t *p = NULL;
    RTP_HEADER * rtp_hdr = rtpHeader;
    NALU_HEADER * nalu_hdr = NULL;

    FU_INDICATOR	*fu_ind = NULL;
    FU_HEADER		*fu_hdr= NULL;

//////////////////////////////////////////////////////////////////////////
//begin rtp_payload and rtp_header

    if ((p = (RTPpacket_t*)malloc (sizeof (RTPpacket_t)))== NULL)
    {
        printf ("RTPpacket_t MMEMORY ERROR\n");
    }

    if ((p->payload = (unsigned char * )malloc (MAXDATASIZE))== NULL)
    {
        printf ("RTPpacket_t payload MMEMORY ERROR\n");
    }

//    printf("版本号 : %d\n",rtp_hdr->version);
    p->v  = rtp_hdr->version;
    p->p  = rtp_hdr->padding;
    p->x  = rtp_hdr->extension;
    p->cc = rtp_hdr->csrc_len;
//    printf("标志位 : %d\n",rtp_hdr->marker);
    p->m = rtp_hdr->marker;
//    printf("负载类型:%d\n",rtp_hdr->payloadtype);
    p->pt = rtp_hdr->payload;
//    printf("包号   : %d \n",rtp_hdr->seq_no);
    p->seq = ntohs(rtp_hdr->seq_no);
//    printf("时间戳 : %d\n",rtp_hdr->timestamp);
    p->timestamp = ntohl(rtp_hdr->timestamp);
//    printf("帧号   : %d\n",rtp_hdr->ssrc);
    p->ssrc = rtp_hdr->ssrc;

//end rtp_payload and rtp_header
//////////////////////////////////////////////////////////////////////////
//begin nal_hdr

    if ((nalu_hdr = (NALU_HEADER * )malloc(sizeof(NALU_HEADER))) == NULL)
    {
        printf("NALU_HEADER MEMORY ERROR\n");
    }

    nalu_hdr =(NALU_HEADER*)&payloadBuffer[0];                        //网络传输过来的字节序 ，当存入内存还是和文档描述的相反，只要匹配网络字节序和文档描述即可传输正确。
//    printf("forbidden_zero_bit: %d\n",nalu_hdr->F);              //网络传输中的方式为：F->NRI->TYPE.. 内存中存储方式为 TYPE->NRI->F (和nal头匹配)。
//    n->forbidden_bit= nalu_hdr->F << 7;                          //内存中的字节序。
////    printf("nal_reference_idc:  %d\n",nalu_hdr->NRI);
//    n->nal_reference_idc = nalu_hdr->NRI << 5;
////    printf("nal 负载类型:       %d\n",nalu_hdr->TYPE);
//    n->nal_unit_type = nalu_hdr->TYPE;

//    fprintf(stderr, "包号   : %d %d %d %d len=%d\n",p->seq, p->timestamp, p->pt, nalu_hdr->TYPE, len);

//end nal_hdr
//////////////////////////////////////////////////////////////////////////
//开始解包
    if ( nalu_hdr->TYPE  == 0)
    {
//        printf("这个包有错误，0无定义 p->seq=%d\n",p->seq);
    }
    else if ( nalu_hdr->TYPE >0 &&  nalu_hdr->TYPE < 24)  //单包
    {
 //        printf("当前包为单包\n");

        if (p->pt == H264)
        {
            memcpy(p->payload, nalu_hdr, 1);
            memcpy(p->payload+1, &payloadBuffer[1], payloadBufferSize-1);
            p->paylen = payloadBufferSize;
        }

        *gotFrame = 1;
    }
    else if ( nalu_hdr->TYPE == 24)                    //STAP-A   单一时间的组合包
    {
        printf("当前包为STAP-A\n");
    }
    else if ( nalu_hdr->TYPE == 25)                    //STAP-B   单一时间的组合包
    {
        printf("当前包为STAP-B\n");
    }
    else if (nalu_hdr->TYPE == 26)                     //MTAP16   多个时间的组合包
    {
        printf("当前包为MTAP16\n");
    }
    else if ( nalu_hdr->TYPE == 27)                    //MTAP24   多个时间的组合包
    {
        printf("当前包为MTAP24\n");
    }
    else if ( nalu_hdr->TYPE == 28)                    //FU-A分片包，解码顺序和传输顺序相同
    {
        fu_ind=(FU_INDICATOR*)&payloadBuffer[0];
//        printf("FU_INDICATOR->F     :%d\n",fu_ind->F);
//        n->forbidden_bit = fu_ind->F << 7;
////        printf("FU_INDICATOR->NRI   :%d\n",fu_ind->NRI);
//        n->nal_reference_idc = fu_ind->NRI << 5;
////        printf("FU_INDICATOR->TYPE  :%d\n",fu_ind->TYPE);
//        n->nal_unit_type = fu_ind->TYPE;

        fu_hdr=(FU_HEADER*)&payloadBuffer[1];
//        printf("FU_HEADER->S        :%d\n",fu_hdr->S);
//        printf("FU_HEADER->E        :%d\n",fu_hdr->E);
//        printf("FU_HEADER->R        :%d\n",fu_hdr->R);
//        printf("FU_HEADER->TYPE     :%d\n",fu_hdr->TYPE);
//        n->nal_unit_type = fu_hdr->TYPE;               //应用的是FU_HEADER的TYPE

        if (p->m == 1)                      //分片包最后一个包
        {
//            printf("当前包为FU-A分片包最后一个包\n");
            memcpy(p->payload, &payloadBuffer[2], payloadBufferSize - 2);
            p->paylen = payloadBufferSize - 2;

            *gotFrame = 1;
        }
        else if (p->m == 0)                 //分片包 但不是最后一个包
        {
            if (fu_hdr->S == 1)                        //分片的第一个包
            {
                unsigned char F;
                unsigned char NRI;
                unsigned char TYPE;
                unsigned char nh;
//                printf("当前包为FU-A分片包第一个包\n");

                F = fu_ind->F << 7;
                NRI = fu_ind->NRI << 5;
                TYPE = fu_hdr->TYPE;    //应用的是FU_HEADER的TYPE
                nh = F | NRI | TYPE;

                memcpy(p->payload, &nh, 1);
                memcpy(p->payload+1, &payloadBuffer[2], payloadBufferSize - 2);
                p->paylen = payloadBufferSize - 2 + 1;
            }
            else                                      //如果不是第一个包
            {
//                printf("当前包为FU-A分片包\n");
                memcpy(p->payload, &payloadBuffer[2], payloadBufferSize - 2);
                p->paylen= payloadBufferSize - 2;
            }
        }
    }
    else if ( nalu_hdr->TYPE == 29)                //FU-B分片包，解码顺序和传输顺序相同
    {
        if (p->m == 1)                  //分片包最后一个包
        {
            printf("当前包为FU-B分片包最后一个包\n");
        }
        else if (p->m == 0)             //分片包 但不是最后一个包
        {
            printf("当前包为FU-B分片包\n");
        }
    }
    else
    {
//        printf("这个包有错误，30-31 没有定义 p->seq=%d\n", p->seq);
    }

//    free (p->payload);
//    free (p);

//结束解包
//////////////////////////////////////////////////////////////////////////

    return p;
}

///**
// * @brief rtp_unpackage
// * @param bufIn
// * @param len
// * @param bufOut
// * @param outLen
// * @param gotFrame 是否得到了一帧的最后一个包
// * @return
// */
//unsigned char * rtp_unpackage(char *bufIn, int len, char *bufOut, int *outLen, int *gotFrame)
//{
//    int bufLen = 0;

//    unsigned char recvbuf[1500] = {0};
//    RTPpacket_t *p = NULL;
//    RTP_HEADER * rtp_hdr = NULL;
//    NALU_HEADER * nalu_hdr = NULL;
//    NALU_t * n  = NULL;
//    FU_INDICATOR	*fu_ind = NULL;
//    FU_HEADER		*fu_hdr= NULL;
//    int total_bytes = 0;                 //当前包传出的数据
//    static int total_recved = 0;         //一共传输的数据

//    memcpy(recvbuf,bufIn, len);          //复制rtp包
////    printf("包长度+ rtp头：   = %d\n",len);

////////////////////////////////////////////////////////////////////////////
////begin rtp_payload and rtp_header

////    p = (RTPpacket_t*)&recvbuf[0];
//    if ((p = (RTPpacket_t*)malloc (sizeof (RTPpacket_t)))== NULL)
//    {
//        printf ("RTPpacket_t MMEMORY ERROR\n");
//    }
//    if ((p->payload = (unsigned char * )malloc (MAXDATASIZE))== NULL)
//    {
//        printf ("RTPpacket_t payload MMEMORY ERROR\n");
//    }


//    rtp_hdr =(RTP_HEADER*)&recvbuf[0];
////    printf("版本号 : %d\n",rtp_hdr->version);
//    p->v  = rtp_hdr->version;
//    p->p  = rtp_hdr->padding;
//    p->x  = rtp_hdr->extension;
//    p->cc = rtp_hdr->csrc_len;
////    printf("标志位 : %d\n",rtp_hdr->marker);
//    p->m = rtp_hdr->marker;
////    printf("负载类型:%d\n",rtp_hdr->payloadtype);
//    p->pt = rtp_hdr->payload;
////    printf("包号   : %d \n",rtp_hdr->seq_no);
//    p->seq = ntohs(rtp_hdr->seq_no);
////    printf("时间戳 : %d\n",rtp_hdr->timestamp);
//    p->timestamp = ntohl(rtp_hdr->timestamp);
////    printf("帧号   : %d\n",rtp_hdr->ssrc);
//    p->ssrc = rtp_hdr->ssrc;

////end rtp_payload and rtp_header
////////////////////////////////////////////////////////////////////////////
////begin nal_hdr
//    if (!(n = AllocNALU()))          //为结构体nalu_t及其成员buf分配空间。返回值为指向nalu_t存储空间的指针
//    {
//        printf("NALU_t MMEMORY ERROR\n");
//    }
//    if ((nalu_hdr = (NALU_HEADER * )malloc(sizeof(NALU_HEADER))) == NULL)
//    {
//        printf("NALU_HEADER MEMORY ERROR\n");
//    }

//    nalu_hdr =(NALU_HEADER*)&recvbuf[12];                        //网络传输过来的字节序 ，当存入内存还是和文档描述的相反，只要匹配网络字节序和文档描述即可传输正确。
////    printf("forbidden_zero_bit: %d\n",nalu_hdr->F);              //网络传输中的方式为：F->NRI->TYPE.. 内存中存储方式为 TYPE->NRI->F (和nal头匹配)。
//    n->forbidden_bit= nalu_hdr->F << 7;                          //内存中的字节序。
////    printf("nal_reference_idc:  %d\n",nalu_hdr->NRI);
//    n->nal_reference_idc = nalu_hdr->NRI << 5;
////    printf("nal 负载类型:       %d\n",nalu_hdr->TYPE);
//    n->nal_unit_type = nalu_hdr->TYPE;

//    fprintf(stderr, "包号   : %d %d %d %d\n",p->seq, p->timestamp, p->pt, nalu_hdr->TYPE);

////end nal_hdr
////////////////////////////////////////////////////////////////////////////
////开始解包
//    if ( nalu_hdr->TYPE  == 0)
//    {
//        printf("这个包有错误，0无定义 p->seq=%d\n",p->seq);
//    }
//    else if ( nalu_hdr->TYPE >0 &&  nalu_hdr->TYPE < 24)  //单包
//    {
////        printf("当前包为单包\n");

//        bufOut[bufLen++] = 0X00;
//        bufOut[bufLen++] = 0X00;
//        bufOut[bufLen++] = 0X00;
//        bufOut[bufLen++] = 0X01;

//        total_bytes +=4;
//        memcpy(p->payload,&recvbuf[13],len-13);
//        p->paylen = len-13;
//        //fwrite(nalu_hdr,1,1,poutfile);
//        memcpy(bufOut+bufLen,nalu_hdr,1);
//        bufLen += 1;

//        total_bytes += 1;
//        //fwrite_number = fwrite(p->payload,1,p->paylen,poutfile);
//        memcpy(bufOut+bufLen,p->payload,p->paylen);
//        bufLen += p->paylen;

//        total_bytes = p->paylen;
////        printf("包长度 + nal= %d\n",total_bytes);

//        *gotFrame = 1;
//    }
//    else if ( nalu_hdr->TYPE == 24)                    //STAP-A   单一时间的组合包
//    {
//        printf("当前包为STAP-A\n");
//    }
//    else if ( nalu_hdr->TYPE == 25)                    //STAP-B   单一时间的组合包
//    {
//        printf("当前包为STAP-B\n");
//    }
//    else if (nalu_hdr->TYPE == 26)                     //MTAP16   多个时间的组合包
//    {
//        printf("当前包为MTAP16\n");
//    }
//    else if ( nalu_hdr->TYPE == 27)                    //MTAP24   多个时间的组合包
//    {
//        printf("当前包为MTAP24\n");
//    }
//    else if ( nalu_hdr->TYPE == 28)                    //FU-A分片包，解码顺序和传输顺序相同
//    {
//        fu_ind=(FU_INDICATOR*)&recvbuf[12];
////        printf("FU_INDICATOR->F     :%d\n",fu_ind->F);
//        n->forbidden_bit = fu_ind->F << 7;
////        printf("FU_INDICATOR->NRI   :%d\n",fu_ind->NRI);
//        n->nal_reference_idc = fu_ind->NRI << 5;
////        printf("FU_INDICATOR->TYPE  :%d\n",fu_ind->TYPE);
//        n->nal_unit_type = fu_ind->TYPE;

//        fu_hdr=(FU_HEADER*)&recvbuf[13];
////        printf("FU_HEADER->S        :%d\n",fu_hdr->S);
////        printf("FU_HEADER->E        :%d\n",fu_hdr->E);
////        printf("FU_HEADER->R        :%d\n",fu_hdr->R);
////        printf("FU_HEADER->TYPE     :%d\n",fu_hdr->TYPE);
//        n->nal_unit_type = fu_hdr->TYPE;               //应用的是FU_HEADER的TYPE

//        if (rtp_hdr->marker == 1)                      //分片包最后一个包
//        {
////            printf("当前包为FU-A分片包最后一个包\n");
//            memcpy(p->payload,&recvbuf[14],len - 14);
//            p->paylen = len - 14;
//            //fwrite_number = fwrite(p->payload,1,p->paylen,poutfile);
//            memcpy(bufOut+bufLen,p->payload,p->paylen);
//            bufLen += p->paylen;

//            total_bytes = p->paylen;
////            printf("包长度 + FU = %d\n",total_bytes);

//            *gotFrame = 1;
//        }
//        else if (rtp_hdr->marker == 0)                 //分片包 但不是最后一个包
//        {
//            if (fu_hdr->S == 1)                        //分片的第一个包
//            {
//                unsigned char F;
//                unsigned char NRI;
//                unsigned char TYPE;
//                unsigned char nh;
////                printf("当前包为FU-A分片包第一个包\n");

//                ///添加4字节起始码
//                bufOut[bufLen++] = 0X00;
//                bufOut[bufLen++] = 0X00;
//                bufOut[bufLen++] = 0X00;
//                bufOut[bufLen++] = 0X01;

//                total_bytes += 4;

//                F = fu_ind->F << 7;
//                NRI = fu_ind->NRI << 5;
//                TYPE = fu_hdr->TYPE;    //应用的是FU_HEADER的TYPE
//                nh = F | NRI | TYPE;

//                memcpy(bufOut+bufLen,&nh,1);
//                bufLen+=1;

//                total_bytes +=1;
//                memcpy(p->payload,&recvbuf[14],len - 14);
//                p->paylen = len - 14;

//                memcpy(bufOut+bufLen,p->payload,p->paylen);
//                bufLen += p->paylen;

//                total_bytes = p->paylen;
////                printf("包长度 + FU_First = %d\n",total_bytes);
//            }
//            else                                      //如果不是第一个包
//            {
////                printf("当前包为FU-A分片包\n");
//                memcpy(p->payload,&recvbuf[14],len - 14);
//                p->paylen= len - 14;

//                memcpy(bufOut+bufLen,p->payload,p->paylen);
//                bufLen += p->paylen;

//                total_bytes = p->paylen;
////                printf("包长度 + FU = %d\n",total_bytes);
//            }
//        }
//    }
//    else if ( nalu_hdr->TYPE == 29)                //FU-B分片包，解码顺序和传输顺序相同
//    {
//        if (rtp_hdr->marker == 1)                  //分片包最后一个包
//        {
//            printf("当前包为FU-B分片包最后一个包\n");

//        }
//        else if (rtp_hdr->marker == 0)             //分片包 但不是最后一个包
//        {
//            printf("当前包为FU-B分片包\n");
//        }
//    }
//    else
//    {
//        printf("这个包有错误，30-31 没有定义 p->seq=%d\n", p->seq);
//    }
//    total_recved += total_bytes;
////    printf("total_recved = %d\n",total_recved);
//    memset(recvbuf,0,1500);
//    free (p->payload);
//    free (p);
//    FreeNALU(n);

//    *outLen = bufLen;

////    fprintf(stderr,"%d %d\n",bufLen,total_bytes);

////结束解包
////////////////////////////////////////////////////////////////////////////

//}


///显示视频数据，此函数不宜做耗时操作，否则会影响播放的流畅性。
void RtpReceiver::doDisplayVideo(const uint8_t *yuv420Buffer, const int &width, const int &height, const int &frameNum)
{
//    fprintf(stderr, "%s %d %d %d\n", __FUNCTION__, yuv420Buffer, width, height);
    if (mVideoCallBack != nullptr)
    {
        VideoFramePtr videoFrame = std::make_shared<VideoFrame>();

        VideoFrame * ptr = videoFrame.get();

        ptr->initBuffer(width, height);
        ptr->setYUVbuf(yuv420Buffer);

        mVideoCallBack->onDisplayVideo(videoFrame, frameNum);
    }
}
