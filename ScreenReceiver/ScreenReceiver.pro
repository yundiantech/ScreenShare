#-------------------------------------------------
#
# Project created by QtCreator 2013-08-24T16:44:37
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ScreenReceiver
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

SOURCES += src/main.cpp \
    src/AppConfig.cpp \
    src/Base/FunctionTransfer.cpp \
    src/Video/ShowVideoWidget.cpp \
    src/Video/VideoEventHandle.cpp \
    src/Video/VideoFrame.cpp \
    src/VideoDecoder/VideoDecoder.cpp \
    src/mainwindow.cpp \
    src/Mutex/Cond.cpp \
    src/Mutex/Mutex.cpp \
    src/RtpReceiver/RtpReceiver.cpp


HEADERS  += src/mainwindow.h \
    src/AppConfig.h \
    src/Base/FunctionTransfer.h \
    src/Mutex/Cond.h \
    src/Mutex/Mutex.h \
    src/RtpReceiver/rtp.h \
    src/Video/ShowVideoWidget.h \
    src/RtpReceiver/RtpReceiver.h \
    src/Video/VideoEventHandle.h \
    src/Video/VideoFrame.h \
    src/VideoDecoder/VideoDecoder.h


FORMS    += src/mainwindow.ui \
    src/Video/ShowVideoWidget.ui


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

