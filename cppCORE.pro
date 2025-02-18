#c++11 support
CONFIG += c++11 

#base settings
QT       -= gui
QT += network
TEMPLATE = lib
TARGET = cppCORE
DEFINES += CPPCORE_LIBRARY
DESTDIR = ../../bin/

#compose version string
SVN_VER= \\\"$$system(cd .. && git describe --tags)\\\"
DEFINES += "CPPCORE_VERSION=$$SVN_VER"

#get decryption key
CRYPT_KEY= \\\"$$cat("CRYPT_KEY.txt", lines)\\\"
DEFINES += "CRYPT_KEY=$$CRYPT_KEY"

#enable O3 optimization
QMAKE_CXXFLAGS_RELEASE -= -O
QMAKE_CXXFLAGS_RELEASE -= -O1
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE *= -O3

#include zlib library
LIBS += -lz

SOURCES += \
    CustomProxyService.cpp \
    Exceptions.cpp \
    HttpRequestHandler.cpp \
    ProxyDataService.cpp \
    Settings.cpp \
    Log.cpp \
    Helper.cpp \
    BasicStatistics.cpp \
    FileWatcher.cpp \
    LinePlot.cpp \
    VersatileFile.cpp \
    VersatileTextStream.cpp \
    WorkerBase.cpp \
    ToolBase.cpp \
    TSVFileStream.cpp \
    ScatterPlot.cpp \
    BarPlot.cpp \
	Histogram.cpp \
    SimpleCrypt.cpp \
    TsvFile.cpp \
	Git.cpp

HEADERS += ToolBase.h \
    CustomProxyService.h \
    Exceptions.h \
    HttpRequestHandler.h \
    ProxyDataService.h \
    Settings.h \
    Log.h \
    Helper.h \
    BasicStatistics.h \
    FileWatcher.h \
    LinePlot.h \
    VersatileFile.h \
    VersatileTextStream.h \
    WorkerBase.h \
    TSVFileStream.h \
    ScatterPlot.h \
    BarPlot.h \
	Histogram.h \
    SimpleCrypt.h \
    TsvFile.h \
	Git.h
	
