#ifndef HTTPREQUESTHANDLER_H
#define HTTPREQUESTHANDLER_H

#include "cppCORE_global.h"
#include <QObject>
#include <QString>
#include <QSslError>
#include <QNetworkAccessManager>
#include <QHttpMultiPart>

using HttpHeaders = QMap<QByteArray, QByteArray>;

struct CPPCORESHARED_EXPORT RequestUrlParams
		: public QMap<QByteArray, QByteArray>
{
	QString asString()
	{
		QString output;
		QMap<QByteArray, QByteArray>::iterator i;
		for (i = this->begin(); i != this->end(); ++i)
		{
			if (!output.isEmpty()) output += "&";
			output += i.key() + "=" + i.value();
		}
		return "?" + output;
	}
};

///Helper class for HTTP(S) communication with webserver
class CPPCORESHARED_EXPORT HttpRequestHandler
		: public QObject
{
	Q_OBJECT

public:
	///Proxy type
	enum ProxyType
	{
		SYSTEM, //from system settings
		INI, //from ini file
		NONE //no proxy
	};

	///Constructor
	HttpRequestHandler(ProxyType proxy_type, QObject* parent=0);

	///Returns basic headers used for all get/post requests. Additional headers that are only used for one request can be given in the get/post methods.
	const HttpHeaders& headers() const;
	///Adds/overrides a basic header.
	void setHeader(const QByteArray& key, const QByteArray& value);
	///Returns headers for a specific file (needed to get the size of a file without fetching its content)
	QMap<QByteArray, QByteArray> head(QString url, const HttpHeaders& add_headers);
	///Performs GET request
	QByteArray get(QString url, const HttpHeaders& add_headers = HttpHeaders());
	///Performs PUT request (modifies existing resources)
	QByteArray put(QString url, const QByteArray& data, const HttpHeaders& add_headers = HttpHeaders());
	///Performs POST request
	QByteArray post(QString url, const QByteArray& data, const HttpHeaders& add_headers = HttpHeaders());
	///Performs POST request for content type multipart
	QByteArray post(QString url, QHttpMultiPart* parts, const HttpHeaders& add_headers = HttpHeaders() );

	HttpRequestHandler() = delete;

signals:
	void proxyAuthenticationRequired(const QNetworkProxy& , QAuthenticator*);

private slots:
	//Handles SSL errors (by ignoring them)
	void handleSslErrors(QNetworkReply*, const QList<QSslError>&);

private:
	QNetworkAccessManager nmgr_;
	HttpHeaders headers_;
	//declared away

};

#endif // HTTPREQUESTHANDLER_H
