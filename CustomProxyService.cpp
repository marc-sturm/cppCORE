#include "CustomProxyService.h"

void CustomProxyService::setProxy(QNetworkProxy custom_proxy)
{
    CustomProxyService& service = instance();
    service.proxy_ = custom_proxy;
}

const QNetworkProxy& CustomProxyService::getProxy()
{
    CustomProxyService& service = instance();
    return service.proxy_;
}

CustomProxyService::CustomProxyService()
{
    proxy_ = QNetworkProxy(QNetworkProxy::NoProxy);
}

CustomProxyService& CustomProxyService::instance()
{
    static CustomProxyService service;
    return service;
}
