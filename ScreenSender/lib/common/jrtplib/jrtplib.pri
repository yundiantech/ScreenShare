
INCLUDEPATH += $$PWD/src

OTHER_FILES += \
    $$PWD/src/rtptypes.h.in \
    $$PWD/src/errcodecommand \
    $$PWD/src/CMakeLists.txt

SOURCES += \
    $$PWD/src/rtpudpv6transmitter.cpp \
    $$PWD/src/rtpudpv4transmitter.cpp \
    $$PWD/src/rtptimeutilities.cpp \
    $$PWD/src/rtpsources.cpp \
    $$PWD/src/rtpsourcedata.cpp \
    $$PWD/src/rtpsessionsources.cpp \
    $$PWD/src/rtpsessionparams.cpp \
    $$PWD/src/rtpsession.cpp \
    $$PWD/src/rtprandomurandom.cpp \
    $$PWD/src/rtprandomrands.cpp \
    $$PWD/src/rtprandomrand48.cpp \
    $$PWD/src/rtprandom.cpp \
    $$PWD/src/rtppollthread.cpp \
    $$PWD/src/rtppacketbuilder.cpp \
    $$PWD/src/rtppacket.cpp \
    $$PWD/src/rtplibraryversion.cpp \
    $$PWD/src/rtpipv6address.cpp \
    $$PWD/src/rtpipv4address.cpp \
    $$PWD/src/rtpinternalsourcedata.cpp \
    $$PWD/src/rtpexternaltransmitter.cpp \
    $$PWD/src/rtperrors.cpp \
    $$PWD/src/rtpdebug.cpp \
    $$PWD/src/rtpcollisionlist.cpp \
    $$PWD/src/rtpbyteaddress.cpp \
    $$PWD/src/rtcpsrpacket.cpp \
    $$PWD/src/rtcpsdespacket.cpp \
    $$PWD/src/rtcpsdesinfo.cpp \
    $$PWD/src/rtcpscheduler.cpp \
    $$PWD/src/rtcprrpacket.cpp \
    $$PWD/src/rtcppacketbuilder.cpp \
    $$PWD/src/rtcppacket.cpp \
    $$PWD/src/rtcpcompoundpacketbuilder.cpp \
    $$PWD/src/rtcpcompoundpacket.cpp \
    $$PWD/src/rtcpbyepacket.cpp \
    $$PWD/src/rtcpapppacket.cpp \
    $$PWD/src/jthread.cpp \
    $$PWD/src/jmutex.cpp

HEADERS  += \
    $$PWD/src/rtpudpv6transmitter.h \
    $$PWD/src/rtpudpv4transmitter.h \
    $$PWD/src/rtptypes_win.h \
    $$PWD/src/rtptypes.h \
    $$PWD/src/rtptransmitter.h \
    $$PWD/src/rtptimeutilities.h \
    $$PWD/src/rtpstructs.h \
    $$PWD/src/rtpsources.h \
    $$PWD/src/rtpsourcedata.h \
    $$PWD/src/rtpsessionsources.h \
    $$PWD/src/rtpsessionparams.h \
    $$PWD/src/rtpsession.h \
    $$PWD/src/rtprawpacket.h \
    $$PWD/src/rtprandomurandom.h \
    $$PWD/src/rtprandomrands.h \
    $$PWD/src/rtprandomrand48.h \
    $$PWD/src/rtprandom.h \
    $$PWD/src/rtppollthread.h \
    $$PWD/src/rtppacketbuilder.h \
    $$PWD/src/rtppacket.h \
    $$PWD/src/rtpmemoryobject.h \
    $$PWD/src/rtpmemorymanager.h \
    $$PWD/src/rtplibraryversion.h \
    $$PWD/src/rtpkeyhashtable.h \
    $$PWD/src/rtpipv6destination.h \
    $$PWD/src/rtpipv6address.h \
    $$PWD/src/rtpipv4destination.h \
    $$PWD/src/rtpipv4address.h \
    $$PWD/src/rtpinternalsourcedata.h \
    $$PWD/src/rtphashtable.h \
    $$PWD/src/rtpexternaltransmitter.h \
    $$PWD/src/rtperrors.h \
    $$PWD/src/rtpdefines.h \
    $$PWD/src/rtpdebug.h \
    $$PWD/src/rtpconfig.h.in \
    $$PWD/src/rtpconfig.h \
    $$PWD/src/rtpcollisionlist.h \
    $$PWD/src/rtpbyteaddress.h \
    $$PWD/src/rtpaddress.h \
    $$PWD/src/rtcpunknownpacket.h \
    $$PWD/src/rtcpsrpacket.h \
    $$PWD/src/rtcpsdespacket.h \
    $$PWD/src/rtcpsdesinfo.h \
    $$PWD/src/rtcpscheduler.h \
    $$PWD/src/rtcprrpacket.h \
    $$PWD/src/rtcppacketbuilder.h \
    $$PWD/src/rtcppacket.h \
    $$PWD/src/rtcpcompoundpacketbuilder.h \
    $$PWD/src/rtcpcompoundpacket.h \
    $$PWD/src/rtcpbyepacket.h \
    $$PWD/src/rtcpapppacket.h \
    $$PWD/src/jthreadconfig.h.in \
    $$PWD/src/jthreadconfig.h \
    $$PWD/src/jthread.h \
    $$PWD/src/jmutexautolock.h \
    $$PWD/src/jmutex.h

win32{
    LIBS += -lAdvapi32
}
