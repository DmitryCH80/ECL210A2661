#-------------------------------------------------
#
# Project created by QtCreator 2017-04-12T09:35:20
#
#-------------------------------------------------
QMAKE_CXXFLAGS += -O0 -large-address-aware

QT       += widgets sql
greaterThan(QT_MAJOR_VERSION, 5): QT += widgets

TARGET = ECL210A2661
TEMPLATE = lib

DEFINES += ECL210A2661_LIBRARY

SOURCES += ECL210A2661.cpp \
    CCommand.cpp \
    General.cpp \
    ECL210A2661Params.cpp

HEADERS += ECL210A2661.h\
        ECL210A2661_global.h \
    CCommand.h \
    General.h \
    CMovedLabel.h \
    ECL210A2661Params.h \
    CTempGraph.h \
    CAxis.h \
    ECL210A2661Config.h \
    CProgress.h \
    ECL210A2661ConfigDlg.h \
    CInfoDialog.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

RESOURCES += \
    resourse.qrc
