QT += core
QT -= gui

CONFIG += c++11

TARGET = Polygon
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    polygon.cpp

HEADERS += \
    polygon.h
