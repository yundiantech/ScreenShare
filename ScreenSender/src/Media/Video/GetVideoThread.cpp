/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#include "AppConfig.h"
#include "GetVideoThread.h"
#include "RtpSender/RtpSenderManager.h"

#if defined __linux
#include <xcb/xcb.h>
#endif

//'1' Use Dshow
//'0' Use VFW
#define USE_DSHOW 0

//Show Dshow Device
static void show_dshow_device()
{
    AVFormatContext *pFormatCtx = avformat_alloc_context();
    AVDictionary* options = nullptr;
    av_dict_set(&options,"list_devices","true",0);
    AVInputFormat *iformat = av_find_input_format("dshow");
    printf("========Device Info=============\n");
    avformat_open_input(&pFormatCtx,"video=dummy",iformat,&options);
    printf("================================\n");
}

//Show Dshow Device Option
static void show_dshow_device_option()
{
    AVFormatContext *pFormatCtx = avformat_alloc_context();
    AVDictionary* options = nullptr;
    av_dict_set(&options,"list_options","true",0);
    AVInputFormat *iformat = av_find_input_format("dshow");
    printf("========Device Option Info======\n");
    avformat_open_input(&pFormatCtx,"video=Integrated Camera",iformat,&options);
    printf("================================\n");
}

//Show VFW Device
static void show_vfw_device()
{
    AVFormatContext *pFormatCtx = avformat_alloc_context();
    AVInputFormat *iformat = av_find_input_format("vfwcap");
    printf("========VFW Device Info======\n");
    avformat_open_input(&pFormatCtx,"list",iformat,nullptr);
    printf("=============================\n");
}

//Show AVFoundation Device
static void show_avfoundation_device()
{
    AVFormatContext *pFormatCtx = avformat_alloc_context();

    AVDictionary* options = nullptr;
    av_dict_set(&options,"list_devices","true",0);
    AVInputFormat *iformat = av_find_input_format("avfoundation");
    printf("==AVFoundation Device Info===\n");
    avformat_open_input(&pFormatCtx, "",iformat, &options);
    printf("=============================\n");
}

GetVideoThread::GetVideoThread()
{
    m_isRun = false;

    pFormatCtx = nullptr;
    outBufferYuv = nullptr;
    pFrame = nullptr;
    pFrameYUV = nullptr;
    pCodecCtx = nullptr;

    m_pause = false;

    mVideoEncoder = new VideoEncoder();

}

GetVideoThread::~GetVideoThread()
{

}

void GetVideoThread::setQuantity(int value)
{
    mVideoEncoder->setQuantity(value);
}

bool GetVideoThread::init()
{

    AVCodec			*pCodec = nullptr;

    pFormatCtx = avformat_alloc_context();

#if defined(WIN32)

    //Show Dshow Device
    show_dshow_device();
    //Show Device Options
    show_dshow_device_option();
    //Show VFW Options
    show_vfw_device();

    AVInputFormat *ifmt = av_find_input_format("dshow"); //使用dshow

    if(avformat_open_input(&pFormatCtx, "video=screen-capture-recorder", ifmt, nullptr)!=0)
    {
        fprintf(stderr, "Couldn't open input stream video.（无法打开输入流）\n");
        return false;
    }

#elif defined __linux
//Linux
//    AVInputFormat *ifmt=av_find_input_format("video4linux2");
//    if(avformat_open_input(&pFormatCtx, "/dev/video0", ifmt, NULL)!=0)
//    {
//        fprintf(stderr, "Couldn't open input stream.\n");
//        return false;
//    }

    ///使用xcb获取屏幕分辨率
    int i, screenNum;
    xcb_connection_t *connection = xcb_connect (nullptr, &screenNum);
    /* Get the screen whose number is screenNum */
    const xcb_setup_t *setup = xcb_get_setup (connection);
    xcb_screen_iterator_t iter = xcb_setup_roots_iterator (setup);

    // we want the screen at index screenNum of the iterator
    for (i = 0; i < screenNum; ++i)
    {
        xcb_screen_next (&iter);
    }
    xcb_screen_t *screen = iter.data;

    /* report */
    printf ("\n");
    printf ("Informations of screen %ld:\n", screen->root);
    printf ("  width.........: %d\n", screen->width_in_pixels);
    printf ("  height........: %d\n", screen->height_in_pixels);
    printf ("  white pixel...: %ld\n", screen->white_pixel);
    printf ("  black pixel...: %ld\n", screen->black_pixel);
    printf ("\n");

    char regionStr[64] = {0};
    sprintf(regionStr, "%dx%d", screen->width_in_pixels, screen->height_in_pixels);

    AVDictionary* options = nullptr;
//    av_dict_set(&options,"list_devices","true", 0);
    /* set frame per second */
//    av_dict_set( &options,"framerate","30", 0);
    av_dict_set( &options,"show_region","1", 0);
    av_dict_set( &options,"video_size", regionStr, 0);
//    av_dict_set( &options, "preset", "medium", 0 );

    /*
    X11 video input device.
    To enable this input device during configuration you need libxcb installed on your system. It will be automatically detected during configuration.
    This device allows one to capture a region of an X11 display.
    refer : https://www.ffmpeg.org/ffmpeg-devices.html#x11grab
    */
    AVInputFormat *ifmt = av_find_input_format("x11grab");
//    if(avformat_open_input(&pFormatCtx, ":0.0+10,10", ifmt, &options) != 0) //从坐标10,10处开始采集
    if(avformat_open_input(&pFormatCtx, ":0.0", ifmt, &options) != 0)
    {
        fprintf(stderr, "\nerror in opening input device\n");
        return false;
    }
#else
    show_avfoundation_device();
    //Mac
    AVInputFormat *ifmt=av_find_input_format("avfoundation");
    //Avfoundation
    //[video]:[audio]
    if(avformat_open_input(&pFormatCtx,"0",ifmt,nullptr)!=0)
    {
        fprintf(stderr, "Couldn't open input stream.\n");
        return false;
    }
#endif

    videoStream = -1;
    pCodecCtx   = nullptr;

    for(unsigned int i=0; i < pFormatCtx->nb_streams; i++)
    {
        if(pFormatCtx->streams[i]->codecpar->codec_type==AVMEDIA_TYPE_VIDEO)
        {
            videoStream = static_cast<int>(i);
            break;
        }
    }

    if(videoStream == -1)
    {
        printf("Didn't find a video stream.（没有找到视频流）\n");
        return false;
    }

//    pCodecCtx = pFormatCtx->streams[videoStream]->codec;

    //find the decoder
    pCodecCtx = avcodec_alloc_context3(nullptr);
    avcodec_parameters_to_context(pCodecCtx, pFormatCtx->streams[videoStream]->codecpar);

    pCodec = avcodec_find_decoder(pCodecCtx->codec_id);

    if(pCodec == nullptr)
    {
        printf("video Codec not found.\n");
        return false;
    }

    if(avcodec_open2(pCodecCtx, pCodec, nullptr)<0)
    {
        printf("Could not open video codec.\n");
        return false;
    }

    pFrame = av_frame_alloc();
    pFrameYUV = av_frame_alloc();

    //***************
//    int Screen_W = GetSystemMetrics(SM_CXSCREEN); //获取屏幕宽高
//    int Screen_H = GetSystemMetrics(SM_CYSCREEN);
    mVideoEncoder->setWidth(pCodecCtx->width, pCodecCtx->height);  //设置编码器的宽高

    return true;
}

void GetVideoThread::deInit()
{
    if (outBufferYuv)
    {
        av_free(outBufferYuv);
        outBufferYuv = nullptr;
    }

    if (pFrame)
    {
        av_free(pFrame);
        pFrame = nullptr;
    }

    if (pFrameYUV)
    {
        av_free(pFrameYUV);
        pFrameYUV = nullptr;
    }

    if (pCodecCtx)
        avcodec_close(pCodecCtx);

    avformat_close_input(&pFormatCtx);
    avformat_free_context(pFormatCtx);

}

void GetVideoThread::startRecord()
{
    m_isRun = true;

    //启动新的线程
    std::thread([&](GetVideoThread *pointer)
    {
        pointer->run();

    }, this).detach();

    mVideoEncoder->startEncode();

}

void GetVideoThread::pauseRecord()
{
    m_pause = true;
}

void GetVideoThread::restoreRecord()
{
    m_getFirst = false;
    m_pause = false;
}

void GetVideoThread::stopRecord()
{
    m_isRun = false;
}

//FILE *fp = fopen("out.yuv","wb");

void GetVideoThread::run()
{
    struct SwsContext *img_convert_ctx = nullptr;

    int y_size = 0;
    int yuvSize = 0;

    if (pCodecCtx)
    {
        ///将数据转成YUV420P格式
        img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
                                         pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P,
                                         SWS_BICUBIC, nullptr, nullptr, nullptr);

        y_size = pCodecCtx->width * pCodecCtx->height;
//        yuvSize = avpicture_get_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height);
        yuvSize = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1);  //按1字节进行内存对齐,得到的内存大小最接近实际大小
//        image_buf_size = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 0);  //按0字节进行内存对齐，得到的内存大小是0
//        image_buf_size = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 4);   //按4字节进行内存对齐，得到的内存大小稍微大一些

        ///理论上 这里的 yuvSize = y_size * 3 / 2

        unsigned int numBytes = static_cast<unsigned int>(yuvSize);
        outBufferYuv = static_cast<uint8_t *>(av_malloc(numBytes * sizeof(uint8_t)));
//        avpicture_fill((AVPicture *) pFrameYUV, outBufferYuv, AV_PIX_FMT_YUV420P,pCodecCtx->width, pCodecCtx->height);
        av_image_fill_arrays(pFrameYUV->data, pFrameYUV->linesize, outBufferYuv, AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1);

    }

    AVPacket packet;

    int64_t firstTime = AppConfig::getTimeStamp_MilliSecond();
    m_getFirst = false;
    int64_t timeIndex = 0;

    bool m_saveVideoFileThread = true;

    while(m_isRun)
    {
        if (av_read_frame(pFormatCtx, &packet)<0)
        {
            fprintf(stderr, "read failed! \n");
            AppConfig::mSleep(10);
            continue;
        }

        if (m_pause)
        {
            av_packet_unref(&packet);
            AppConfig::mSleep(10);
            continue;
        }
//fprintf(stderr, "%s %d %d \n", __FUNCTION__, packet.stream_index, videoStream);
        if(packet.stream_index == videoStream)
        {
            int64_t time = 0;
            if (m_saveVideoFileThread)
            {
                if (m_getFirst)
                {
                    int64_t secondTime = AppConfig::getTimeStamp_MilliSecond();
                    time = secondTime - firstTime + timeIndex;
                }
                else
                {
                    firstTime = AppConfig::getTimeStamp_MilliSecond();
                    timeIndex = 0;
                    m_getFirst = true;
                }
            }

            if (avcodec_send_packet(pCodecCtx, &packet) != 0)
            {
               fprintf(stderr, "input AVPacket to decoder failed!\n");
               av_packet_unref(&packet);
               continue;
            }

            while (0 == avcodec_receive_frame(pCodecCtx, pFrame))
            {
                /// 转换成YUV420
                /// 由于解码后的数据不一定是yuv420p，比如硬件解码后会是yuv420sp，因此这里统一转成yuv420p
                sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameYUV->data, pFrameYUV->linesize);

                if (m_saveVideoFileThread)
                {
                    uint8_t * picture_buf = static_cast<uint8_t *>(av_malloc(static_cast<size_t>(yuvSize)));
                    memcpy(picture_buf, outBufferYuv, static_cast<size_t>(yuvSize));
//                        fwrite(picture_buf,1,y_size*3/2,fp);
//                        av_free(picture_buf);

                    mVideoEncoder->inputYuvBuffer(picture_buf, yuvSize, time); //将yuv数据添加到h.264编码的线程
                }
            }
        }
        else
        {
            fprintf(stderr, "other %d \n", packet.stream_index);
        }

        av_packet_unref(&packet);

    }

    sws_freeContext(img_convert_ctx);

    fprintf(stderr, "record stopping... \n");

    m_pause = false;

    deInit();

    mVideoEncoder->stopEncode();

    fprintf(stderr, "record finished! \n");

}

