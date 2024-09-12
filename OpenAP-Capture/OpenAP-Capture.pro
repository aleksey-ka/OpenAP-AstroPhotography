#-------------------------------------------------
#
# Project created by QtCreator 2020-10-28T13:53:51
#
#-------------------------------------------------

QT += core gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = OpenAP-Capture
TEMPLATE = app

CONFIG += c++17

SOURCES += \
        Hardware.Camera.cpp \
		Hardware.Camera.Mock.cpp \
		Hardware.Camera.ZWO.ASI.cpp \
        Hardware.FilterWheel.cpp \
		Hardware.FilterWheel.ZWO.EFW.cpp \
		Hardware.Focuser.Mock.cpp \
        Hardware.Focuser.cpp \
		Hardware.Focuser.DIY.ArduinoFocuser.cpp \
		Hardware.Focuser.ZWO.EAF.cpp \
        Image.Debayer.CFA.cpp \
        Image.Debayer.HalfRes.cpp \
        Image.Debayer.HQLinear.cpp \
        Image.Formats.cpp \
		Image.Image.cpp \
        Image.Math.cpp \
		Image.Math.Advanced.cpp \
        Image.RawImage.cpp \
		Image.Stack.cpp \
        ImageView.cpp \
        Math.Geometry.cpp \
        Math.LinearAlgebra.cpp \
        Main.cpp \
        MainFrame.cpp \
        MainFrame.Tools.cpp \
        PaintView.cpp \
        Renderer.cpp \

HEADERS += \
        Hardware.Camera.h \
		Hardware.Camera.Mock.h \
		Hardware.Camera.ZWO.ASI.h \
        Hardware.FilterWheel.h \
		Hardware.FilterWheel.ZWO.EFW.h \
		Hardware.Focuser.Mock.h \
        Hardware.Focuser.h \
		Hardware.Focuser.DIY.ArduinoFocuser.h \
		Hardware.Focuser.ZWO.EAF.h \
        Image.Debayer.h \
        Image.Debayer.CFA.h \
        Image.Debayer.HalfRes.h \
        Image.Debayer.HQLinear.h \
        Image.Formats.h \
		Image.Image.h \
        Image.Math.h \
		Image.Math.Advanced.h \
        Image.RawImage.h \
		Image.Stack.h \
        Image.Qt.h \
        ImageView.h \
        Math.Geometry.h \
        Math.LinearAlgebra.h \
        Math.Matrix.h \
        MainFrame.h \
        MainFrame.Tools.h \
        PaintView.h \
        Renderer.h

FORMS += \
        MainFrame.ui

win32: {
    INCLUDEPATH += "..\..\ASI SDK\include"
    LIBS += -lASICamera2 -lEFW_filter -lEAF_focuser
    RC_ICONS += MainFrame.ico
}
unix: {
    INCLUDEPATH += /usr/include/libasi
    LIBS += -lASICamera2 -lEFWFilter -lEAFFocuser
    QMAKE_CXXFLAGS += -std=c++17 #CONFIG alone does not work with neither gcc nor clang
    #        -Wno-psabi #suppress GCC7.1 warnings
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
