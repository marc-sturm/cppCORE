#include "ProxyDataService.h"
#include "Settings.h"
#include "Exceptions.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>


ProxyDataService::ProxyDataService()
{
	initProxy();
}

bool ProxyDataService::setCredentials(QString user, QString password)
{
	QNetworkProxy tmp_proxy;
	tmp_proxy.setType(QNetworkProxy::HttpProxy);
	tmp_proxy.setHostName(proxy_.hostName());
	tmp_proxy.setPort(proxy_.port());
	tmp_proxy.setUser(user);
	tmp_proxy.setPassword(password);

	if(test_connection(tmp_proxy))
	{
		proxy_ = tmp_proxy;
		qDebug() << "proxy settings applied";
	}

	qDebug() << "Failed to connect with provided proxy settings.";
	return false;
}


void ProxyDataService::initProxy()
{
	//test no proxy
	if(test_connection(QNetworkProxy(QNetworkProxy::NoProxy)))
	{
		qDebug() << "No proxy required.";
		proxy_ = QNetworkProxy(QNetworkProxy::NoProxy);
		return;
	}


	//test system proxy
	QNetworkProxyQuery npq(QUrl("http://www.google.com"));
	QList<QNetworkProxy> system_proxies = QNetworkProxyFactory::systemProxyForQuery(npq);
	if(system_proxies.size() > 0)
	{
		if(system_proxies.size() > 1) qDebug() << "Multiple proxy settings found.";
		foreach(const QNetworkProxy system_proxy, system_proxies)
		{
			//debug
			qDebug() << system_proxy.hostName() << system_proxy.port() << system_proxy.user() << system_proxy.password();
			if(test_connection(system_proxy))
			{
				qDebug() << "System proxy used.";
				proxy_ = system_proxy;
				return;
			}
		}
	}


	//test proxy from ini
	QNetworkProxy tmp_proxy;
	tmp_proxy.setType(QNetworkProxy::HttpProxy);
	tmp_proxy.setHostName(Settings::string("proxy_host"));
	tmp_proxy.setPort(Settings::integer("proxy_port"));
	tmp_proxy.setUser(Settings::string("proxy_user", true));
	tmp_proxy.setPassword(Settings::string("proxy_password", true));

	if(test_connection(tmp_proxy))
	{
		qDebug() << "Proxy settings from INI file used.";
		proxy_ = tmp_proxy;

		return;
	}

	if(tmp_proxy.password().isEmpty() || tmp_proxy.user().isEmpty())
	{
		qDebug() << "Incomplete proxy settings from INI file used.";
		proxy_ = tmp_proxy;
	}

	THROW(ArgumentException, "Proxy settings provided, but couldn't connect to the internet.")

}

bool ProxyDataService::test_connection(const QNetworkProxy& proxy)
{
	QNetworkAccessManager network_manager;
	network_manager.setProxy(proxy);
	QNetworkRequest request;
	request.setUrl(QUrl("http://www.google.com"));
	QNetworkReply* reply = network_manager.get(request);

	qDebug() << reply->readAll();
	qDebug() << reply->error();

	return reply->error()==QNetworkReply::NoError;
}
