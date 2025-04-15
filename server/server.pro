QT += network multimedia widgets
CONFIG += c++17

TARGET = Server
TEMPLATE = app

SOURCES += \
    main.cpp
    server.cpp \
    clientconnection.cpp

HEADERS += \
    server.h \
    clientconnection.h

# Дополнительные настройки
DEFINES += QT_DEPRECATED_WARNINGS
QMAKE_CXXFLAGS += $$QMAKE_CFLAGS_WARN_ON