#-------------------------------------------------
#
# Project created by QtCreator 2018-08-11T23:22:19
#
#-------------------------------------------------

# gui we need for Vector3D
QT       += network gui

TARGET = gw2api
TEMPLATE = lib
CONFIG += staticlib

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
        gw2api.cpp \
        gw2mumblefile.cpp \
    gw2account.cpp \
    gw2character.cpp

HEADERS += \
        gw2api.h \
    gw2mumblefile.h \
    gw2account.h \
    gw2common.h \
    gw2character.h
unix {
    target.path = /usr/lib
    INSTALLS += target
}
