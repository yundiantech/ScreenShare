/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#include "AppConfig.h"
#include "GetAudioThread.h"
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

GetAudioThread::GetAudioThread()
{
    m_isRun = false;

    pFormatCtx = nullptr;
    aCodecCtx = nullptr;
    aFrame = nullptr;

    m_pause = false;

    aFrame = nullptr;
    aFrame_ReSample = nullptr;

    audio_buf_size_L = 0;
    audio_buf_size_R = 0;

    mAudioEncoder = new AudioEncoder();

}

GetAudioThread::~GetAudioThread()
{

}

void GetAudioThread::setQuantity(int value)
{
//    mVideoEncoder->setQuantity(value);
}

bool GetAudioThread::init()
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

    if(avformat_open_input(&pFormatCtx, "audio=virtual-audio-capturer", ifmt, nullptr)!=0)
//    if(avformat_open_input(&pFormatCtx, "audio=@device_cm_{33D9A762-90C8-11D0-BD43-00A0C911CE86}\\wave_{DA3EBB84-3C1D-47F9-A2A8-BCD7CC9AA4E7}", ifmt, nullptr)!=0)
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
//        return -1;
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
        return VideoOpenFailed;
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
        return VideoOpenFailed;
    }
#endif

    audioStream = -1;
    aCodecCtx   = nullptr;

    for(unsigned int i=0; i < pFormatCtx->nb_streams; i++)
    {
        if(pFormatCtx->streams[i]->codecpar->codec_type==AVMEDIA_TYPE_AUDIO)
        {
            audioStream = static_cast<int>(i);
            break;
        }
    }

    if(audioStream == -1)
    {
        printf("Didn't find a audio stream.（没有找到音频流）\n");
        return false;
    }

//    pCodecCtx = pFormatCtx->streams[videoStream]->codec;

    //find the decoder
    aCodecCtx = avcodec_alloc_context3(nullptr);
    avcodec_parameters_to_context(aCodecCtx, pFormatCtx->streams[audioStream]->codecpar);

    pCodec = avcodec_find_decoder(aCodecCtx->codec_id);

    if(pCodec == nullptr)
    {
        printf("audio Codec not found.\n");
        return false;
    }

    if(avcodec_open2(aCodecCtx, pCodec, nullptr)<0)
    {
        printf("Could not open audio codec.\n");
        return false;
    }

    ///解码音频相关
    aFrame = av_frame_alloc();

    initResample();

    return true;
}

void GetAudioThread::deInit()
{
//    if (outBufferYuv)
//    {
//        av_free(outBufferYuv);
//        outBufferYuv = nullptr;
//    }

//    if (pFrame)
//    {
//        av_free(pFrame);
//        pFrame = nullptr;
//    }

//    if (pFrameYUV)
//    {
//        av_free(pFrameYUV);
//        pFrameYUV = nullptr;
//    }

//    if (pCodecCtx)
//        avcodec_close(pCodecCtx);

//    avformat_close_input(&pFormatCtx);
//    avformat_free_context(pFormatCtx);

}

void GetAudioThread::startRecord()
{
    m_isRun = true;

    //启动新的线程
    std::thread([&](GetAudioThread *pointer)
    {
        pointer->run();

    }, this).detach();

}

void GetAudioThread::pauseRecord()
{
    m_pause = true;
}

void GetAudioThread::restoreRecord()
{
    m_getFirst = false;
    m_pause = false;
}

void GetAudioThread::stopRecord()
{
    m_isRun = false;
}

bool GetAudioThread::initResample()
{
    //重采样设置选项-----------------------------------------------------------start
    aFrame_ReSample = nullptr;

    //frame->16bit 44100 PCM 统一音频采样格式与采样率
    swrCtx = nullptr;

    //输入的声道布局
    int in_ch_layout;

    /// 由于ffmpeg编码aac需要输入FLTP格式的数据。
    /// 因此这里将音频重采样成44100 双声道  AV_SAMPLE_FMT_FLTP
    //重采样设置选项----------------
    //输入的采样格式
    in_sample_fmt = aCodecCtx->sample_fmt;
    //输出的采样格式 16bit PCM
    out_sample_fmt = AV_SAMPLE_FMT_FLTP;
    //输入的采样率
    in_sample_rate = aCodecCtx->sample_rate;
    //输入的声道布局
    in_ch_layout = aCodecCtx->channel_layout;

    //输出的采样率
    out_sample_rate = 44100;

    //输出的声道布局
    out_ch_layout = AV_CH_LAYOUT_STEREO;
    audio_tgt_channels = av_get_channel_layout_nb_channels(out_ch_layout);

//        //输出的声道布局
//        out_ch_layout = av_get_default_channel_layout(audio_tgt_channels); ///AV_CH_LAYOUT_STEREO
//        out_ch_layout &= ~AV_CH_LAYOUT_STEREO;

    if (in_ch_layout <= 0)
    {
        if (aCodecCtx->channels == 2)
        {
            in_ch_layout = AV_CH_LAYOUT_STEREO;
        }
        else
        {
            in_ch_layout = AV_CH_LAYOUT_MONO;
        }
    }

    swrCtx = swr_alloc_set_opts(nullptr, out_ch_layout, out_sample_fmt, out_sample_rate,
                                         in_ch_layout, in_sample_fmt, in_sample_rate, 0, nullptr);

    /** Open the resampler with the specified parameters. */
    int ret = swr_init(swrCtx);
    if (ret < 0)
    {
        char buff[128]={0};
        av_strerror(ret, buff, 128);

        fprintf(stderr, "Could not open resample context %s\n", buff);
        swr_free(&swrCtx);
        swrCtx = nullptr;

        return false;
    }

    return true;
}

void GetAudioThread::run()
{
    struct SwsContext *img_convert_ctx = nullptr;

    AVPacket packet;

    mAudioPts = 0;

    mAudioEncoder->openEncoder();

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

        if(packet.stream_index == audioStream)
        {
            if (int ret = avcodec_send_packet(aCodecCtx, &packet) && ret != 0)
            {
               char buffer[1024] = {0};
               av_strerror(ret, buffer, 1024);
               fprintf(stderr, "input AVPacket to decoder failed! ret = %d %s\n", ret, buffer);
            }
            else
            {
            //    while (0 == avcodec_receive_frame(pCodecCtx, pFrame))
                while(1)
                {
                    int ret = avcodec_receive_frame(aCodecCtx, aFrame);
                    if (ret != 0)
                    {
            //            char buffer[1024] = {0};
            //            av_strerror(ret, buffer, 1024);
            //            fprintf(stderr, "avcodec_receive_frame = %d %s\n", ret, buffer);
                        break;
                    }

                    ///解码一帧后才能获取到采样率等信息，因此将初始化放到这里
                    if (aFrame_ReSample == nullptr)
                    {
                        aFrame_ReSample = av_frame_alloc();

                        int nb_samples = av_rescale_rnd(swr_get_delay(swrCtx, out_sample_rate) + aFrame->nb_samples, out_sample_rate, in_sample_rate, AV_ROUND_UP);

    //                    av_samples_fill_arrays(aFrame_ReSample->data, aFrame_ReSample->linesize, audio_buf_resample, audio_tgt_channels, aFrame_ReSample->nb_samples, out_sample_fmt, 0);

                        aFrame_ReSample->format = out_sample_fmt;
                        aFrame_ReSample->channel_layout = out_ch_layout;
                        aFrame_ReSample->sample_rate = out_sample_rate;
                        aFrame_ReSample->nb_samples = nb_samples;

                        ret = av_frame_get_buffer(aFrame_ReSample, 0);
                        if (ret < 0)
                        {
                            fprintf(stderr, "Error allocating an audio buffer\n");
                            exit(1);
                        }
                    }

                    ///执行重采样
                    int len2 = swr_convert(swrCtx, aFrame_ReSample->data, aFrame_ReSample->nb_samples, (const uint8_t**)aFrame->data, aFrame->nb_samples);

    ///下面这两种方法计算的大小是一样的
    #if 0
                    int resampled_data_size = len2 * audio_tgt_channels * av_get_bytes_per_sample(out_sample_fmt);
    #else
                    int resampled_data_size = av_samples_get_buffer_size(NULL, audio_tgt_channels, aFrame_ReSample->nb_samples, out_sample_fmt, 1);
    #endif

                    int OneChannelDataSize = resampled_data_size / audio_tgt_channels;


//fprintf(stderr, "OneChannelDataSize=%d %d %d\n", OneChannelDataSize, mAudioEncoder->getONEFrameSize(), aFrame->nb_samples);
/// 由于平面模式的pcm存储方式为：LLLLLLLLLLLLLLLLLLLLLRRRRRRRRRRRRRRRRRRRRR，
/// 因此这里需要将左右声道数据分开存入文件才可以正常播放。
/// 使用播放器单独播放左右 声道数据测试即可(以单声道 44100 32bit打开播放)。
//static FILE *fp1 = fopen("out-L.pcm", "wb");
//fwrite(aFrame_ReSample->data[0], 1, OneChannelDataSize, fp1);
//if (audio_tgt_channels >= 2)
//{
//    static FILE *fp2 = fopen("out-R.pcm", "wb");
//    fwrite(aFrame_ReSample->data[1], 1, OneChannelDataSize, fp2);
//}

                    dealWithAudioFrame(OneChannelDataSize);
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

    mAudioEncoder->closeEncoder();

    fprintf(stderr, "record finished! \n");

}

void GetAudioThread::dealWithAudioFrame(const int &OneChannelDataSize)
{
    ///编码器一帧的采样为1024，而这里一次获取到的不是1024，因此需要放入队列，然后每次从队里取1024次采样交给编码器。
    ///PS：平面模式的pcm存储方式为：LLLLLLLLLLLLLLLLLLLLLRRRRRRRRRRRRRRRRRRRRR，需要了解这句话的含义。

    memcpy(audio_buf_L + audio_buf_size_L, aFrame_ReSample->data[0], OneChannelDataSize);
    audio_buf_size_L += OneChannelDataSize;

    if (audio_tgt_channels >= 2)
    {
        memcpy(audio_buf_R + audio_buf_size_R, aFrame_ReSample->data[1], OneChannelDataSize);
        audio_buf_size_R += OneChannelDataSize;
    }

    int index = 0;
    int ONEAudioSize = mAudioEncoder->getONEFrameSize();

    int leftSize  = audio_buf_size_L;

    ONEAudioSize /= audio_tgt_channels;

    ///由于采集到的数据很大，而编码器一次只需要很少的数据。
    ///因此将采集到的数据分成多次传给编码器。
    /// 由于平面模式的pcm存储方式为：LLLLLLLLLLLLLLLLLLLLLRRRRRRRRRRRRRRRRRRRRR，因此这里合并完传给编码器就行了
    while(1)
    {
        if (leftSize >= ONEAudioSize)
        {
            uint8_t * buffer = (uint8_t *)malloc(ONEAudioSize * audio_tgt_channels);
            memcpy(buffer, audio_buf_L+index, ONEAudioSize);

            if (audio_tgt_channels >= 2)
            {
                memcpy(buffer+ONEAudioSize, audio_buf_R+index, ONEAudioSize);
            }

            ///一秒44100采样，一帧1024次采样，那么一帧的时间就是： 1000 / (44100 / 1024) = 23.2199580毫秒
            mAudioPts += 23.2199580;

            AACFramePtr aacFrame = mAudioEncoder->encode((uint8_t*)buffer, ONEAudioSize * audio_tgt_channels);
            uint32_t time = mAudioPts;

            if (aacFrame != nullptr)
            {
                AppConfig::gRtpSenderManager->sendAudio(aacFrame, time);
            }

            free(buffer);

            index    += ONEAudioSize;
            leftSize -= ONEAudioSize;
        }
        else
        {
            if (leftSize > 0)
            {
                memcpy(audio_buf_L, audio_buf_L+index, leftSize);
                memcpy(audio_buf_R, audio_buf_R+index, leftSize);
            }
            audio_buf_size_L = leftSize;
            audio_buf_size_R = leftSize;
            break;
        }
    }
}
