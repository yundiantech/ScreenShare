# 从零开始学习音视频编程技术（47）接收并播放rtp流  

这是Qt的工程，建议使用Qt Creator 打开

版本更新日志：  
【V1.0.0】
Qt5.6.2(vs2013/mingw) + ffmpeg4.1 + jrtplib3.9  
1.程序可以在linux下直接编译使用。  
2.已经编译好的库为centos7.4(64位)(gcc 版本 4.8.5)下编译的，如需32位的库请自行编译。 
3.支持使用jrtplib接收rtp流，通过RtpReceiver.h中的宏#define USE_JRTPLIB 1 来开关。  


# 注:只实现了h264数据解析，其他暂未实现。  

关于代码的解释，请访问:http://blog.yundiantech.com/?log=blog&id=47  
学习音视频技术欢迎访问 http://blog.yundiantech.com  
音视频技术交流讨论欢迎加 QQ群 121376426  

