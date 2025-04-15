QT += network multimedia widgets
CONFIG += c++17

TARGET = Client
TEMPLATE = app

SOURCES += \
    clientwindow.cpp \
    main.cpp

HEADERS += \
    clientwindow.h

FORMS += \
    clientwindow.ui

# Дополнительные настройки
DEFINES += QT_DEPRECATED_WARNINGS
QMAKE_CXXFLAGS += $$QMAKE_CFLAGS_WARN_ON

# Для правильного отображения иконок (если используются)
RC_ICONS = icons/app_icon.ico