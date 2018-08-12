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
    gw2/account/gw2account.cpp \
    gw2/gw2currentplayer.cpp \
    mapview.cpp \
    inventoryview.cpp \
    gw2/characters/gw2characters.cpp \
    manualquery.cpp \
    gw2/characters/gw2character.cpp

HEADERS += \
        overview.h \
    gw2/account/gw2account.h \
    mapview.h \
    inventoryview.h \
    gw2/characters/gw2characters.h \
    manualquery.h \
    gw2/characters/gw2character.h \
    gw2/gw2currentplayer.h

FORMS += \
        overview.ui

# api lib here
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../gw2api/release/ -lgw2api
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../gw2api/debug/ -lgw2api
else:unix: LIBS += -L$$OUT_PWD/../gw2api/ -lgw2api

INCLUDEPATH += $$PWD/../gw2api
DEPENDPATH += $$PWD/../gw2api

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../gw2api/release/libgw2api.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../gw2api/debug/libgw2api.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../gw2api/release/gw2api.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../gw2api/debug/gw2api.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../gw2api/libgw2api.a
