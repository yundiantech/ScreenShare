# 从零开始学习音视频编程技术（45）采集屏幕打包发送RTP  
ffmpeg4.1采集屏幕编码264和265并打包发送rtp  

这是Qt的工程，建议使用Qt Creator 打开

版本更新日志：  
【V1.0.0】Qt5.6.2(vs2013/mingw) + ffmpeg4.1   
1.采集屏幕编码成h264.  
2.使用系统socket直接发送打包好的rtp包。  

【V1.1.0】
Qt5.6.2(vs2013/mingw) + ffmpeg4.1 + jrtplib3.9  
1.程序可以在linux下直接编译使用。  
2.已经编译好的库为centos7.4(64位)(gcc 版本 4.8.5)下编译的，如需32位的库请自行编译。 
3.打包发送rtp采用了jrtplib打包发送和直接组包发送，通过RtpSender.h中的宏#define USE_JRTPLIB 1 来开关。  


# 注:只实现了h264数据打包rtp，h265暂未实现。  

关于代码的解释，请访问:http://blog.yundiantech.com/?log=blog&id=45  
学习音视频技术欢迎访问 http://blog.yundiantech.com  
音视频技术交流讨论欢迎加 QQ群 121376426  

