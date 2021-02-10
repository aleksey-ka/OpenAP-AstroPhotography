#-------------------------------------------------
#
# Project created by QtCreator 2020-10-28T13:53:51
#
#-------------------------------------------------

QT += core gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MyAstroCapture
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        asicamera.cpp \
        filterwheel.cpp \
        focuser.cpp \
        image.cpp \
        image.formats.cpp \
        imageview.cpp \
        main.cpp \
        mainframe.cpp \
        mockcamera.cpp \
        renderer.cpp \
        renderer.hqlinear.cpp

HEADERS += \
        asicamera.h \
        camera.h \
        filterwheel.h \
        focuser.h \
        image.formats.h \
        image.h \
        imageview.h \
        mainframe.h \
        mockcamera.h \
        renderer.h

FORMS += \
        mainframe.ui

win32: {
    INCLUDEPATH += "..\ASI SDK\include"
    LIBS += -lASICamera2 -lEFW_filter
} else: unix: {
    INCLUDEPATH += /usr/include/libasi
    LIBS += -lASICamera2 -lEFWFilter
}

win32:contains(QMAKE_HOST.arch, x86_64) {  
    LIBS += -L"..\ASI SDK\lib\x64"
} else {
    LIBS += -L"..\ASI SDK\lib\x86"
}

RESOURCES += resources.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
