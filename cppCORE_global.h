#ifndef CPPCORE_GLOBAL_H
#define CPPCORE_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(CPPCORE_LIBRARY)
#  define CPPCORESHARED_EXPORT Q_DECL_EXPORT
#else
#  define CPPCORESHARED_EXPORT Q_DECL_IMPORT
#endif

// #define SET_QT_OFFSCREEN() qputenv("QT_QPA_PLATFORM", "offscreen")

#ifdef USE_CORE_APP
#define APP_BASE_CLASS QCoreApplication
#else
#define APP_BASE_CLASS QApplication
#endif


#endif // CPPCORE_GLOBAL_H
