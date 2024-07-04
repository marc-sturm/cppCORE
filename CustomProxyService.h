#ifndef CUSTOMPROXYSERVICE_H
#define CUSTOMPROXYSERVICE_H

#include "cppCORE_global.h"
#include <QNetworkProxy>
#include <QObject>


class CPPCORESHARED_EXPORT CustomProxyService
    : public QObject
{
    Q_OBJECT
public:
    static void setProxy(QNetworkProxy custom_proxy);
    static const QNetworkProxy& getProxy();

private:
    QNetworkProxy proxy_;

    CustomProxyService();
    static CustomProxyService& instance();
};

#endif // CUSTOMPROXYSERVICE_H
