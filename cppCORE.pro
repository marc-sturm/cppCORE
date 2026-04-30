include("../lib.pri")

#base settings
QT       += gui widgets charts
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
    BarPlot.cpp \
    CustomProxyService.cpp \
    Exceptions.cpp \
    Histogram.cpp \
    HttpRequestHandler.cpp \
    LinePlot.cpp \
    LoggingWorker.cpp \
    PlotUtils.cpp \
    ProxyDataService.cpp \
    ScatterPlot.cpp \
    Settings.cpp \
    Log.cpp \
    Helper.cpp \
    BasicStatistics.cpp \
    FileWatcher.cpp \
    VersatileFile.cpp \
    VersatileTextStream.cpp \
    WorkerBase.cpp \
    ToolBase.cpp \
    TSVFileStream.cpp \
    SimpleCrypt.cpp \
    TsvFile.cpp \
    Git.cpp

HEADERS += ToolBase.h \
    BarPlot.h \
    CustomProxyService.h \
    Exceptions.h \
    GzipStreamDecompressor.h \
    Histogram.h \
    HttpRequestHandler.h \
    LinePlot.h \
    LoggingWorker.h \
    PlotUtils.h \
    ProxyDataService.h \
    ScatterPlot.h \
    Settings.h \
    Log.h \
    Helper.h \
    BasicStatistics.h \
    FileWatcher.h \
    VersatileFile.h \
    VersatileTextStream.h \
    WorkerBase.h \
    TSVFileStream.h \
    SimpleCrypt.h \
    TsvFile.h \
    Git.h
	

RESOURCES += \
    cppCORE.qrc
