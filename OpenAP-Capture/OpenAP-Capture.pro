#-------------------------------------------------
#
# Project created by QtCreator 2020-10-28T13:53:51
#
#-------------------------------------------------

QT += core gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = OpenAP-Capture
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
        Hardware.Camera.cpp \
        Hardware.Camera.MockCamera.cpp \
        Hardware.Camera.ZWO.ASICamera.cpp \
        Hardware.FilterWheel.cpp \
        Hardware.FilterWheel.ZWO.EFWheel.cpp \
        Hardware.Focuser.cpp \
        Hardware.Focuser.DIYFocuser.cpp \
        Hardware.Focuser.ZWO.EAFocuser.cpp \
        Image.Debayer.CFA.cpp \
        Image.Debayer.HalfRes.cpp \
        Image.Debayer.HQLinear.cpp \
        Image.Formats.cpp \
        Image.Math.cpp \
		Image.Math.Advanced.cpp \
        Image.RawImage.cpp \
        Image.Image.cpp \
        ImageView.cpp \
        Math.Geometry.cpp \
        Math.LinearAlgebra.cpp \
        Main.cpp \
        MainFrame.cpp \
        MainFrame.Tools.cpp \
        Renderer.cpp \

HEADERS += \
        Hardware.Camera.h \
        Hardware.Camera.MockCamera.h \
        Hardware.Camera.ZWO.ASICamera.h \
        Hardware.FilterWheel.h \
        Hardware.FilterWheel.ZWO.EFWheel.h \
        Hardware.Focuser.h \
        Hardware.Focuser.DIYFocuser.h \
        Hardware.Focuser.ZWO.EAFocuser.h \
        Image.Debayer.h \
        Image.Debayer.CFA.h \
        Image.Debayer.HalfRes.h \
        Image.Debayer.HQLinear.h \
        Image.Formats.h \
        Image.Math.h \
		Image.Math.Advanced.h \
        Image.RawImage.h \
        Image.Image.h \
        Image.Qt.h \
        ImageView.h \
        Math.Geometry.h \
        Math.LinearAlgebra.h \
        Math.Matrix.h \
        MainFrame.h \
        MainFrame.Tools.h \
        Renderer.h

FORMS += \
        MainFrame.ui

win32: {
    INCLUDEPATH += "..\..\ASI SDK\include"
	LIBS += -lASICamera2 -lEFW_filter -lEAF_focuser
    RC_ICONS += MainFrame.ico
} else: unix: {
    INCLUDEPATH += /usr/include/libasi
    LIBS += -lASICamera2 -lEFWFilter -lEAFFocuser
}

win32:contains(QMAKE_HOST.arch, x86_64) {  
    LIBS += -L"..\..\ASI SDK\lib\x64"
} else {
    LIBS += -L"..\..\ASI SDK\lib\x86"
}

RESOURCES += Resources.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
