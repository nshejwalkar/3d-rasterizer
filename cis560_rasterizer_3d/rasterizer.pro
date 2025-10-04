QT       += core gui

CONFIG += c++11

INCLUDEPATH += include
INCLUDEPATH += $$PWD

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = cis277_hw01
TEMPLATE = app

*-clang*|*-g++* {
    message("Enabling additional warnings")
    QMAKE_CXXFLAGS += -fms-extensions
}

SOURCES += main.cpp\
    camera.cpp \
        mainwindow.cpp \
    polygon.cpp \
    rasterizer.cpp \
    tiny_obj_loader.cc

HEADERS  += mainwindow.h \
    camera.h \
    constants.h \
    debug.h \
    polygon.h \
    rasterizer.h \
    tiny_obj_loader.h

FORMS    += mainwindow.ui
