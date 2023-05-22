#include "ProxyDataService.h"
#include "Settings.h"
#include "Exceptions.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QThread>
#include <QTimer>


ProxyDataService::ProxyDataService()
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

		return;
	}

	THROW(NetworkException, "Proxy settings provided, but couldn't connect to the internet.")
}

bool ProxyDataService::setCredentials(QString user, QString password)
{
	ProxyDataService& service = instance();
	QNetworkProxy tmp_proxy;
	tmp_proxy.setType(QNetworkProxy::HttpProxy);
	tmp_proxy.setHostName(service.proxy_.hostName());
	tmp_proxy.setPort(service.proxy_.port());
	tmp_proxy.setUser(user);
	tmp_proxy.setPassword(password);

	if(test_connection(tmp_proxy))
	{
		service.proxy_ = tmp_proxy;
		qDebug() << "proxy settings applied";
		return true;
	}

	qDebug() << "Failed to connect with provided proxy settings.";
	return false;
}

const QNetworkProxy& ProxyDataService::getProxy()
{
	ProxyDataService& service = instance();
	return service.proxy_;
}

bool ProxyDataService::isConnected()
{
	QNetworkProxy proxy = getProxy();
	return test_connection(proxy);
}

ProxyDataService& ProxyDataService::instance()
{
	static ProxyDataService service;
	return service;
}

bool ProxyDataService::test_connection(QNetworkProxy proxy)
{
	QNetworkAccessManager network_manager;
	network_manager.setProxy(proxy);
	QNetworkReply* reply = network_manager.get(QNetworkRequest(QUrl("https://www.google.com")));

	//make the loop process the reply immediately
	QEventLoop loop;
	connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));

	// add shorter time out for NoProxy check
	QTimer timeout_timer;
	if(proxy.type() == QNetworkProxy::NoProxy)
	{
		timeout_timer.setSingleShot(true);
		connect(&timeout_timer, SIGNAL(timeout()), &loop, SLOT(quit()));
		timeout_timer.start(3000);
	}

	loop.exec();

	if((proxy.type() == QNetworkProxy::NoProxy) && !timeout_timer.isActive())
	{
		// reply ran in time out
		qDebug() << "Network timeout for NoProxy check!";
		return false;
	}

	qDebug() << reply->readAll();
	qDebug() << reply->error();
	qDebug() << (reply->error() == QNetworkReply::NoError);

	return (reply->error() == QNetworkReply::NoError);

//	QEventLoop loop;
//	QTimer timer;
//	timer.setSingleShot(true);
//	connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
//	connect(&network_manager, SIGNAL(proxyAuthenticationRequired(const QNetworkProxy& , QAuthenticator*)), &loop, SLOT(quit()));
//	timer.start(10000);
//	loop.exec();
//	network_manager.
//	QNetworkReply*reply = network_manager.get(QNetworkRequest(QUrl("https://www.google.com")));
//	connect(reply, SIGNAL(finished), &loop, SLOT(quit()));

//	if(timer.isActive())
//	{
//		//no timeout -> analyse reply
//		timer.stop();
//		qDebug() << reply->readAll();
//		qDebug() << reply->error();
//		int status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
//		//cleanup
//		disconnect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
//		disconnect(reply, SIGNAL(finished), &loop, SLOT(quit()));
//		delete reply;

//		//return depending on status code
//		return ((status_code >= 200) && (status_code < 300));
//	}
//	else
//	{
//		//timeout
//		qDebug() << "Internet check failed (time out)!";
//		//cleanup
//		disconnect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
//		disconnect(reply, SIGNAL(finished), &loop, SLOT(quit()));
//		delete reply;

//		return false;
//	}

//	QTimer timer;
//	timer.setInterval(3000);
//	connect(&timer, SIGNAL(timeout()), request, SLOT())
//	timer.start()
//	network_manager.set


//	qDebug() << reply->readAll();
//	qDebug() << reply->error();

//	return reply->error()==QNetworkReply::NoError;

}
