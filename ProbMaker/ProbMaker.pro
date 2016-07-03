#-------------------------------------------------
#
# Project created by QtCreator 2016-05-15T00:49:53
#
#-------------------------------------------------

QT       += core gui widgets

CONFIG += c++14

TARGET = ProbMaker
TEMPLATE = app


SOURCES += main.cpp\
        probmaker.cpp \
    voronoidiagrammaker.cpp \
    fieldmaker.cpp

HEADERS  += probmaker.h \
    voronoidiagrammaker.h \
    fieldmaker.h

FORMS    += probmaker.ui

LIBS += -L/usr/local/lib `pkg-config --libs opencv`
LIBS += -lboost_system -lboost_thread

unix:!macx: LIBS += -L$$OUT_PWD/../Polygon/ -lPolygon
INCLUDEPATH += $$PWD/../Polygon
DEPENDPATH += $$PWD/../Polygon


unix:!macx: LIBS += -L$$OUT_PWD/../DisplayAnswer/ -lDisplayAnswer
INCLUDEPATH += $$PWD/../DisplayAnswer
DEPENDPATH += $$PWD/../DisplayAnswer
