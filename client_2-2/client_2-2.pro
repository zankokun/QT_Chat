QT += core gui multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
DEFINES += MEDIA_SUPPORT

CONFIG += c++17

SOURCES += main.cpp

HEADERS += header.h

LIBS += -lws2_32

QT += core gui
