#include "HttpRequestHandler.h"
#include "Exceptions.h"
#include "Settings.h"
#include "Helper.h"

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QNetworkProxy>

#include <QAuthenticator>
#include <QFile>
#include <QPointer>
#include <QHttpMultiPart>

HttpRequestHandler::HttpRequestHandler(ProxyType proxy_type, QObject* parent)
	: QObject(parent)
	, nmgr_()
	, headers_()
{

	//default headers
	setHeader("User-Agent", "GSvar");
	setHeader("X-Custom-User-Agent", "GSvar");

	//proxy
	if (proxy_type==SYSTEM)
	{
		QNetworkProxyFactory::setUseSystemConfiguration(true);
	}
	else if (proxy_type==INI)
	{
		QNetworkProxy proxy;
		proxy.setType(QNetworkProxy::HttpProxy);
		proxy.setHostName(Settings::string("proxy_host"));
		proxy.setPort(Settings::integer("proxy_port"));
		nmgr_.setProxy(proxy);
	}
	else
	{
		nmgr_.setProxy(QNetworkProxy(QNetworkProxy::NoProxy));
	}

	connect(&nmgr_, SIGNAL(sslErrors(QNetworkReply*, const QList<QSslError> &)), this, SLOT(handleSslErrors(QNetworkReply*, const QList<QSslError>&)));
	if (parent != nullptr)
	{
		parent->connect(&nmgr_, SIGNAL(proxyAuthenticationRequired(const QNetworkProxy& , QAuthenticator*)), parent, SLOT(handleProxyAuthentification(const QNetworkProxy& , QAuthenticator*)));
	}
}

const HttpHeaders& HttpRequestHandler::headers() const
{
	return headers_;
}

void HttpRequestHandler::setHeader(const QByteArray& key, const QByteArray& value)
{
	headers_.insert(key, value);
}

QMap<QByteArray, QByteArray> HttpRequestHandler::head(QString url, const HttpHeaders& add_headers)
{
	QMap<QByteArray, QByteArray> output;
	//request
	QNetworkRequest request;
	request.setUrl(url);
	for(auto it=headers_.begin(); it!=headers_.end(); ++it)
	{
		request.setRawHeader(it.key(), it.value());
	}
	for(auto it=add_headers.begin(); it!=add_headers.end(); ++it)
	{
		request.setRawHeader(it.key(), it.value());
	}

	//query
	QNetworkReply* reply = nmgr_.head(request);

	//make the loop process the reply immediately
	QEventLoop loop;
	connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
	loop.exec();

	QList<QByteArray> header_list = reply->rawHeaderList();
	for (int i = 0; i < header_list.size(); i++)
	{
		output.insert(header_list.value(i), reply->rawHeader(header_list.value(i)));
	}

	if (reply->error()!=QNetworkReply::NoError)
	{
		THROW(Exception, "Network error " + QString::number(reply->error()) + "\nError message: " + reply->errorString());
	}

	reply->deleteLater();
	return output;
}

QByteArray HttpRequestHandler::get(QString url, const HttpHeaders& add_headers)
{
	//request
	QNetworkRequest request;
	request.setUrl(url);
	for(auto it=headers_.begin(); it!=headers_.end(); ++it)
	{
		request.setRawHeader(it.key(), it.value());
	}
	for(auto it=add_headers.begin(); it!=add_headers.end(); ++it)
	{
		request.setRawHeader(it.key(), it.value());
	}
	request.setRawHeader("User-Agent", "Qt NetworkAccess 1.3");
	request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);

	QByteArray output;
	int retry_attempts = 5;
	bool needs_retry = false;
	for (int i = 0; i < retry_attempts; i++)
	{
		//query
		QNetworkReply* reply = nmgr_.get(request);

		//make the loop process the reply immediately
		QEventLoop loop;
		connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
		loop.exec();

		//output
		reply->setReadBufferSize(0);
		output = reply->readAll();
		if (reply->error()!=QNetworkReply::NoError)
		{
			if (i == retry_attempts - 1)
			{
				THROW(Exception, "Network error " + QString::number(reply->error()) + "\nError message: " + reply->errorString() + "\nReply: " + output);
			}
			else
			{
				needs_retry = true;
			}
		}

		reply->deleteLater();
		if (!needs_retry)
		{
			break;
		}
	}
	return output;
}

QByteArray HttpRequestHandler::put(QString url, const QByteArray& data, const HttpHeaders& add_headers)
{
	//request
	QNetworkRequest request;
	request.setUrl(url);
	for(auto it=headers_.begin(); it!=headers_.end(); ++it)
	{
		request.setRawHeader(it.key(), it.value());
	}
	for(auto it=add_headers.begin(); it!=add_headers.end(); ++it)
	{
		request.setRawHeader(it.key(), it.value());
	}

	//query
	QNetworkReply* reply = nmgr_.put(request, data);

	//make the loop process the reply immediately
	QEventLoop loop;
	connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
	loop.exec();

	//output
	QByteArray output = reply->readAll();
	if (reply->error()!=QNetworkReply::NoError)
	{
		THROW(Exception, "Network error " + QString::number(reply->error()) + "\nError message: " + reply->errorString() + "\nReply: " + output);
	}
	reply->deleteLater();
	return output;
}

QByteArray HttpRequestHandler::post(QString url, const QByteArray& data, const HttpHeaders& add_headers)
{
	//request
	QNetworkRequest request;
	request.setUrl(url);
	for(auto it=headers_.begin(); it!=headers_.end(); ++it)
	{
		request.setRawHeader(it.key(), it.value());
	}
	for(auto it=add_headers.begin(); it!=add_headers.end(); ++it)
	{
		request.setRawHeader(it.key(), it.value());
	}

	//query
	QNetworkReply* reply = nmgr_.post(request, data);

	//make the loop process the reply immediately
	QEventLoop loop;
	connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
	loop.exec();

	//output
	QByteArray output = reply->readAll();
	if (reply->error()!=QNetworkReply::NoError)
	{
		THROW(Exception, "Network error " + QString::number(reply->error()) + "\nError message: " + reply->errorString() + "\nReply: " + output);
	}
	reply->deleteLater();
	return output;
}

QByteArray HttpRequestHandler::post(QString url, QHttpMultiPart* parts, const HttpHeaders& add_headers)
{
	//request
	QNetworkRequest request;
	request.setUrl(url);
	for(auto it=headers_.begin(); it!=headers_.end(); ++it)
	{
		request.setRawHeader(it.key(), it.value());
	}
	for(auto it=add_headers.begin(); it!=add_headers.end(); ++it)
	{
		request.setRawHeader(it.key(), it.value());
	}

	//query
	QNetworkReply* reply = nmgr_.post(request, parts);

	//make the loop process the reply immediately
	QEventLoop loop;
	connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
	loop.exec();

	//output
	QByteArray output = reply->readAll();
	if (reply->error()!=QNetworkReply::NoError)
	{
		THROW(Exception, "Network error " + QString::number(reply->error()) + "\nError message: " + reply->errorString() + "\nReply: " + output);
	}
	reply->deleteLater();
	return output;
}

void HttpRequestHandler::handleSslErrors(QNetworkReply* reply, const QList<QSslError>& errors)
{
	foreach(const QSslError& error, errors)
	{
		qDebug() << "ignore error" << error.errorString();
	}
	reply->ignoreSslErrors(errors);
}

