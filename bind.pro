TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp

HEADERS += \
    bind.hpp

QMAKE_CXXFLAGS += -std=c++1y
