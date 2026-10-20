QT       += core gui widgets sql

CONFIG   += c++17
CONFIG   -= console

TEMPLATE = app
TARGET   = university_system

SOURCES += \
    login.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    login.h \
    mainwindow.h

FORMS += \
    mainwindow.ui
