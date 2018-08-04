#-------------------------------------------------
#
# Project created by QtCreator 2018-07-30T13:01:41
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = gw2things
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


SOURCES += \
        main.cpp \
        overview.cpp \
    gw2/gw2mumblefile.cpp \
    gw2/gw2api.cpp \
    gw2/account/gw2account.cpp \
    gw2/gw2currentplayer.cpp \
    mapview.cpp

HEADERS += \
        overview.h \
    gw2/gw2mumblefile.h \
    gw2/gw2api.h \
    gw2/account/gw2account.h \
    gw2/gw2currentplayer.h \
    mapview.h

FORMS += \
        overview.ui
