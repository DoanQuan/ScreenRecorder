QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    AppController/appcontroller.cpp \
    AppController/recordscreen.cpp \
    RecordController/ffmpegrecorderimpl.cpp \
    main.cpp

HEADERS += \
    AppController/appcontroller.h \
    AppController/recordscreen.h \
    RecordController/ffmpegrecorderimpl.h \
    RecordController/recorderiface.h

FORMS +=

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32: LIBS += -L$$PWD/RecordController/lib/lib/ -lavcodec

INCLUDEPATH += $$PWD/RecordController/lib/include/libavcodec
DEPENDPATH += $$PWD/RecordController/lib/include/libavcodec

win32: LIBS += -L$$PWD/RecordController/lib/lib/ -lavdevice

INCLUDEPATH += $$PWD/RecordController/lib/include
DEPENDPATH += $$PWD/RecordController/lib/include

win32: LIBS += -L$$PWD/RecordController/lib/lib/ -lavfilter

INCLUDEPATH += $$PWD/RecordController/lib/include
DEPENDPATH += $$PWD/RecordController/lib/include

win32: LIBS += -L$$PWD/RecordController/lib/lib/ -lavformat

INCLUDEPATH += $$PWD/RecordController/lib/include
DEPENDPATH += $$PWD/RecordController/lib/include

win32: LIBS += -L$$PWD/RecordController/lib/lib/ -lavutil

INCLUDEPATH += $$PWD/RecordController/lib/include
DEPENDPATH += $$PWD/RecordController/lib/include

win32: LIBS += -L$$PWD/RecordController/lib/lib/ -lswresample

INCLUDEPATH += $$PWD/RecordController/lib/include
DEPENDPATH += $$PWD/RecordController/lib/include

win32: LIBS += -L$$PWD/RecordController/lib/lib/ -lswscale

INCLUDEPATH += $$PWD/RecordController/lib/include
DEPENDPATH += $$PWD/RecordController/lib/include

LIBS += -lOle32
