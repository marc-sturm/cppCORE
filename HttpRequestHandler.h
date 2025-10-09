#ifndef HTTPREQUESTHANDLER_H
#define HTTPREQUESTHANDLER_H

#include "cppCORE_global.h"
#include <QObject>
#include <QString>
#include <QSslError>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QHttpMultiPart>

using HttpHeaders = QMap<QByteArray, QByteArray>;

struct CPPCORESHARED_EXPORT RequestUrlParams
		: public QMap<QByteArray, QByteArray>
{
	QString asString()
	{
		QString output;
		QMap<QByteArray, QByteArray>::iterator i;
		for (i = begin(); i != end(); ++i)
		{
			if (!output.isEmpty()) output += "&";
			output += i.key() + "=" + i.value();
		}
		return "?" + output;
	}
};

struct CPPCORESHARED_EXPORT ServerReply
{
    int status_code = 200;
    QMap<QByteArray, QByteArray> headers;
    QByteArray body;
};

///Helper class for HTTP(S) communication with webserver
class CPPCORESHARED_EXPORT HttpRequestHandler
		: public QObject
{
	Q_OBJECT

public:

	///Constructor
	HttpRequestHandler(QNetworkProxy proxy, QObject* parent=0);

	///Returns basic headers used for all get/post requests. Additional headers that are only used for one request can be given in the get/post methods.
	const HttpHeaders& headers() const;
	///Adds/overrides a basic header.
	void setHeader(const QByteArray& key, const QByteArray& value);
	///Returns headers for a specific file (needed to get the size of a file without fetching its content)
    ServerReply head(QString url, const HttpHeaders& add_headers);
	///Performs GET request
    ServerReply get(QString url, const HttpHeaders& add_headers = HttpHeaders());
	///Performs PUT request (modifies existing resources)
    ServerReply put(QString url, const QByteArray& data, const HttpHeaders& add_headers = HttpHeaders());
    ///Performs POST request
    ServerReply post(QString url, const QByteArray& data, const HttpHeaders& add_headers = HttpHeaders());
    ///Performs POST request for content type multipart
    ServerReply post(QString url, QHttpMultiPart* parts, const HttpHeaders& add_headers = HttpHeaders());

	HttpRequestHandler() = delete;

signals:
	void proxyAuthenticationRequired(const QNetworkProxy& , QAuthenticator*);

private slots:
	//Handles SSL errors (by ignoring them)
	void handleSslErrors(QNetworkReply*, const QList<QSslError>&);

private:
    QString networkErrorAsString(QNetworkReply::NetworkError error);
	QNetworkAccessManager nmgr_;
	HttpHeaders headers_;
	//declared away

};

#endif // HTTPREQUESTHANDLER_H
