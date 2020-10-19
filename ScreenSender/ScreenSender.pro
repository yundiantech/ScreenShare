#-------------------------------------------------
#
# Project created by QtCreator 2019-10-27T14:42:56
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ScreenSender
TEMPLATE = app

UI_DIR  = obj/Gui
MOC_DIR = obj/Moc
OBJECTS_DIR = obj/Obj


#将输出文件直接放到源码目录下的bin目录下，将dll都放在了次目录中，用以解决运行后找不到dll的问
#DESTDIR=$$PWD/bin/
contains(QT_ARCH, i386) {
    message("32-bit")
    DESTDIR = $${PWD}/bin32
} else {
    message("64-bit")
    DESTDIR = $${PWD}/bin64
}

SOURCES += \
        src/AppConfig.cpp \
        src/Media/Audio/AudioEncoder.cpp \
        src/Media/Audio/AudioFrame/AACFrame.cpp \
        src/Media/Audio/AudioFrame/PCMFrame.cpp \
        src/Media/Audio/GetAudioThread.cpp \
        src/Media/MediaReader.cpp \
        src/Mutex/Cond.cpp \
        src/Mutex/Mutex.cpp \
        src/NALU/nalu.cpp \
        src/RtpSender/RtpSender.cpp \
        src/RtpSender/RtpSenderManager.cpp \
        src/Media/Video/GetVideoThread.cpp \
        src/Media/Video/VideoEncoder.cpp \
        src/main.cpp \
        src/mainwindow.cpp \
    src/Media/Video/VideoFrame/VideoFrame.cpp

HEADERS += \
    src/AppConfig.h \
    src/Media/Audio/AudioEncoder.h \
    src/Media/Audio/AudioFrame/AACFrame.h \
    src/Media/Audio/AudioFrame/PCMFrame.h \
    src/Media/Audio/GetAudioThread.h \
    src/Media/MediaReader.h \
    src/Mutex/Cond.h \
    src/Mutex/Mutex.h \
    src/NALU/h264.h \
    src/NALU/h265.h \
    src/NALU/nalu.h \
    src/RtpSender/RtpSender.h \
    src/RtpSender/RtpSenderManager.h \
    src/RtpSender/rtp.h \
    src/Media/Video/GetVideoThread.h \
    src/Media/Video/VideoEncoder.h \
    src/mainwindow.h \
    src/Media/Video/VideoFrame/VideoFrame.h

win32{

    include($$PWD/lib/common/jrtplib/jrtplib.pri)

    contains(QT_ARCH, i386) {
        message("32-bit")
        INCLUDEPATH += $$PWD/lib/win32/ffmpeg/include \
                       $$PWD/src

        LIBS += -L$$PWD/lib/win32/ffmpeg/lib -lavcodec -lavdevice -lavfilter -lavformat -lavutil -lpostproc -lswresample -lswscale

    } else {
        message("64-bit")
        INCLUDEPATH += $$PWD/lib/win64/ffmpeg/include \
                       $$PWD/src

        LIBS += -L$$PWD/lib/win64/ffmpeg/lib -lavcodec -lavdevice -lavfilter -lavformat -lavutil -lpostproc -lswresample -lswscale

    }

    LIBS += -lws2_32

}

unix{
    contains(QT_ARCH, i386) {
        message("32-bit, 请自行编译32位库!")
    } else {
        message("64-bit")
        INCLUDEPATH += $$PWD/src \
                       $$PWD/lib/linux/ffmpeg/include \
                       $$PWD/lib/linux/jrtplib/include/jrtplib3


        LIBS += -L$$PWD/lib/linux/ffmpeg/lib  -lavformat  -lavcodec -lavdevice -lavfilter -lavutil -lswresample -lswscale -lpostproc

        LIBS += -L$$PWD/lib/linux/jrtplib/lib -ljrtp

        LIBS += -lpthread -ldl -lxcb
    }

#QMAKE_POST_LINK 表示编译后执行内容
#QMAKE_PRE_LINK 表示编译前执行内容

#解压库文件
#QMAKE_PRE_LINK += "cd $$PWD/lib/linux && tar xvzf ffmpeg.tar.gz "
system("cd $$PWD/lib/linux && tar xvzf ffmpeg.tar.gz")
system("cd $$PWD/lib/linux && tar xvzf jrtplib.tar.gz")

}

FORMS += \
    src/mainwindow.ui
