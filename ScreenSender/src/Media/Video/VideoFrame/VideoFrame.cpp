#include "VideoFrame.h"

VideoFrame::VideoFrame()
{
    mNalu = nullptr;
}

VideoFrame::~VideoFrame()
{
    NALUParsing::FreeNALU(mNalu); //释放NALU内存
    mNalu = nullptr;
}

void VideoFrame::setNalu(uint8_t *buffer, const int &len, const T_NALU_TYPE &type)
{
    T_NALU *nalu = NALUParsing::AllocNALU(len, type);

    nalu->type = type;

    if (type == T_NALU_H264)
    {
        T_H264_NALU_HEADER *nalu_header = (T_H264_NALU_HEADER *)(buffer);

        nalu->nalu.h264Nalu.startcodeprefix_len = 4;      //! 4 for parameter sets and first slice in picture, 3 for everything else (suggested)
        nalu->nalu.h264Nalu.len = len;                 //! Length of the NAL unit (Excluding the start code, which does not belong to the NALU)
        nalu->nalu.h264Nalu.forbidden_bit = 0;            //! should be always FALSE
        nalu->nalu.h264Nalu.nal_reference_idc = nalu_header->NRI;        //! NALU_PRIORITY_xxxx
        nalu->nalu.h264Nalu.nal_unit_type = nalu_header->TYPE;            //! NALU_TYPE_xxxx
        nalu->nalu.h264Nalu.lost_packets = false;  //! true, if packet loss is detected
        memcpy(nalu->nalu.h264Nalu.buf, buffer, len);  //! contains the first byte followed by the EBSP

//        {
//            char *bufTmp = (char*)(Buf + StartCode);
//            char s[10];
//            itoa(bufTmp[0], s, 2);
//            fprintf(stderr, "%s %08s %x %d\n", __FUNCTION__, s, bufTmp[0] , nalu_header->TYPE);
//        }
    }
    else
    {
//        T_H265_NALU_HEADER *nalu_header = (T_H265_NALU_HEADER *)(Buf + StartCode);

//        nalu->nalu.h265Nalu.startCodeLen = StartCode;      //! 4 for parameter sets and first slice in picture, 3 for everything else (suggested)
//        nalu->nalu.h265Nalu.len = naluSize;                 //! Length of the NAL unit (Excluding the start code, which does not belong to the NALU)
//        nalu->nalu.h265Nalu.h265NaluHeader = *nalu_header;
//        memcpy(nalu->nalu.h265Nalu.buf, Buf, naluSize);  //! contains the first byte followed by the EBSP

//        {
//            char *bufTmp = (char*)(Buf);
//            fprintf(stderr, "%s %02x%02x%02x%02x%02x%02x %d %d\n", __FUNCTION__, bufTmp[0], bufTmp[1], bufTmp[2], bufTmp[3], bufTmp[4], bufTmp[5], nalu->nalu.h265Nalu.h265NaluHeader.nal_unit_type, nalu_header->nal_unit_type);
//        }
    }

    if (mNalu != nullptr)
    {
        NALUParsing::FreeNALU(mNalu); //释放NALU内存
    }

    mNalu = nalu;
}
