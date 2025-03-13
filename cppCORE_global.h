#ifndef CPPCORE_GLOBAL_H
#define CPPCORE_GLOBAL_H

#include <QtCore/qglobal.h>
#include <algorithm>

#if defined(CPPCORE_LIBRARY)
#  define CPPCORESHARED_EXPORT Q_DECL_EXPORT
#else
#  define CPPCORESHARED_EXPORT Q_DECL_IMPORT
#endif

#ifdef QT_VERSION
    #if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        #define QT_ENDL Qt::endl
        #define QT_SKIP_EMPTY_PARTS Qt::SkipEmptyParts
        #define QLIST_SWAP_ITEMS_AT(list, i, j) list.swapItemsAt(i, j)
    #else
        #define QT_ENDL endl
        #define QT_SKIP_EMPTY_PARTS QString::SkipEmptyParts
        #define QLIST_SWAP_ITEMS_AT(list, i, j) list.swap(i, j)
    #endif
#endif

template <typename T>
QSet<T> LIST_TO_SET(const QList<T>& list)
{
    #if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        return QSet<T>(list.begin(), list.end());
    #else
        return list.toSet();
    #endif
}

template <typename T>
QList<T> SET_TO_LIST(const QSet<T>& set)
{
    QList<T> list;
    #if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        list = QList<T>(set.begin(), set.end());
    #else
        list = set.toList();
    #endif
    std::sort(list.begin(), list.end());
    return list;
}


template <typename T>
T SIZE_TO_INT(const T& size_value)
{
#if QT_VERSION > QT_VERSION_CHECK(5, 15, 0)
    return size_value;
#else
    return static_cast<int>(size_value)
#endif
}

#endif // CPPCORE_GLOBAL_H
