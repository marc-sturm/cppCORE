include("../lib.pri")

#base settings
QT += gui widgets charts httpserver
QT += network
TARGET = cppCORE
DEFINES += CPPCORE_LIBRARY

#compose version string
GIT_VERSION = \\\"$$system(cd .. && git describe --tags)\\\"
DEFINES += "GIT_VERSION=$$GIT_VERSION"
GIT_DATE = \\\"$$system(cd .. && git log -1 --format=%cs)\\\"
DEFINES += "GIT_DATE=$$GIT_DATE"

#get decryption key
CRYPT_KEY= \\\"$$cat("CRYPT_KEY.txt", lines)\\\"
DEFINES += "CRYPT_KEY=$$CRYPT_KEY"

SOURCES += \
    BarPlot.cpp \
    BasicServer.cpp \
    Exceptions.cpp \
    Histogram.cpp \
    HttpRequestHandler.cpp \
    LinePlot.cpp \
    LoggingWorker.cpp \
    PlotUtils.cpp \
    ScatterPlot.cpp \
    Settings.cpp \
    Log.cpp \
    Helper.cpp \
    BasicStatistics.cpp \
    FileWatcher.cpp \
    VersatileFile.cpp \
    VersatileTextStream.cpp \
    ToolBase.cpp \
    TSVFileStream.cpp \
    SimpleCrypt.cpp \
    TsvFile.cpp \
    Git.cpp \
    ProxyCredentialsHandler.cpp

HEADERS += ToolBase.h \
    BarPlot.h \
    BasicServer.h \
    Exceptions.h \
    GzipStreamDecompressor.h \
    Histogram.h \
    HttpRequestHandler.h \
    LinePlot.h \
    LoggingWorker.h \
    PlotUtils.h \
    ScatterPlot.h \
    Settings.h \
    Log.h \
    Helper.h \
    BasicStatistics.h \
    FileWatcher.h \
    VersatileFile.h \
    VersatileTextStream.h \
    TSVFileStream.h \
    SimpleCrypt.h \
    TsvFile.h \
    Git.h \
    ProxyCredentialsHandler.h
	

RESOURCES += \
    cppCORE.qrc
