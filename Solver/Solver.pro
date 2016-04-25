QT += core
QT -= gui

CONFIG += c++11

TARGET = Solver
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    Algorithm/simplealgorithm.cpp \
    algorithmwrapper.cpp

HEADERS += \
    Algorithm/simplealgorithm.h \
    algorithmwrapper.h
