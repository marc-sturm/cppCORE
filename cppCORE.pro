include("../lib.pri")

#base settings
QT       -= gui
QT += network
TARGET = cppCORE
DEFINES += CPPCORE_LIBRARY

#compose version string
SVN_VER= \\\"$$system(cd .. && git describe --tags)\\\"
DEFINES += "CPPCORE_VERSION=$$SVN_VER"

#get decryption key
CRYPT_KEY= \\\"$$cat("CRYPT_KEY.txt", lines)\\\"
DEFINES += "CRYPT_KEY=$$CRYPT_KEY"

SOURCES += \
    CustomProxyService.cpp \
    Exceptions.cpp \
    HttpRequestHandler.cpp \
    LoggingWorker.cpp \
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
    LoggingWorker.h \
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
	
