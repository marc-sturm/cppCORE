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
CRYPT_KEY= \\\"$$cat("CRYPT_KEY.txt", blob)\\\"
DEFINES += "CRYPT_KEY=$$CRYPT_KEY"

#enable O3 optimization
QMAKE_CXXFLAGS_RELEASE -= -O
QMAKE_CXXFLAGS_RELEASE -= -O1
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE *= -O3

SOURCES += \
    Exceptions.cpp \
    HttpRequestHandler.cpp \
    Settings.cpp \
    Log.cpp \
    Helper.cpp \
    BasicStatistics.cpp \
    FileWatcher.cpp \
    LinePlot.cpp \
    WorkerBase.cpp \
    ToolBase.cpp \
    TSVFileStream.cpp \
    ScatterPlot.cpp \
    BarPlot.cpp \
	Histogram.cpp \
    SimpleCrypt.cpp \
    TsvFile.cpp

HEADERS += ToolBase.h \
    Exceptions.h \
    HttpRequestHandler.h \
    Settings.h \
    Log.h \
    Helper.h \
    BasicStatistics.h \
    FileWatcher.h \
    LinePlot.h \
    WorkerBase.h \
    TSVFileStream.h \
    ScatterPlot.h \
    BarPlot.h \
	Histogram.h \
    SimpleCrypt.h \
    TsvFile.h
	
