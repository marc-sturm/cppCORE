#include "HttpRequestHandler.h"
#include "Exceptions.h"
#include "Settings.h"
#include "Helper.h"

#include <QEventLoop>
#include <QNetworkProxy>
#include <QAuthenticator>
#include <QFile>
#include <QPointer>
#include <QHttpMultiPart>
#include "CustomProxyService.h"

HttpRequestHandler::HttpRequestHandler(QNetworkProxy proxy, QObject* parent)
	: QObject(parent)
	, nmgr_()
	, headers_()
{

	//default headers
	setHeader("User-Agent", "GSvar");
	setHeader("X-Custom-User-Agent", "GSvar");

    //proxy settings
    nmgr_.setProxy(proxy);

    //override existing proxy, if custom proxy settings have been provided
    if (CustomProxyService::getProxy() != QNetworkProxy::NoProxy)
    {
        nmgr_.setProxy(CustomProxyService::getProxy());
    }

	connect(&nmgr_, SIGNAL(sslErrors(QNetworkReply*, const QList<QSslError> &)), this, SLOT(handleSslErrors(QNetworkReply*, const QList<QSslError>&)));
}

const HttpHeaders& HttpRequestHandler::headers() const
{
	return headers_;
}

void HttpRequestHandler::setHeader(const QByteArray& key, const QByteArray& value)
{
	headers_.insert(key, value);
}

ServerReply HttpRequestHandler::head(QString url, const HttpHeaders& add_headers)
{
    ServerReply output;

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
        output.headers.insert(header_list[i], reply->rawHeader(header_list[i]));
    }

    output.status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if (reply->error()!=QNetworkReply::NoError)
    {
        THROW_HTTP(HttpException, "HTTP Error: " + networkErrorAsString(reply->error()) + "\nIODevice Error: " + reply->errorString(), output.status_code, output.headers, output.body);
    }

    reply->deleteLater();
    return output;
}

ServerReply HttpRequestHandler::get(QString url, const HttpHeaders& add_headers)
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
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, true);

    ServerReply output;
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
        QList<QByteArray> header_list = reply->rawHeaderList();

        output = ServerReply{};
        for (int i = 0; i < header_list.size(); i++)
        {
            output.headers.insert(header_list[i], reply->rawHeader(header_list[i]));
        }
        output.status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        output.body = reply->readAll();
        if ((reply->error()!=QNetworkReply::NoError) && (reply->error()!=QNetworkReply::RemoteHostClosedError))
        {
            if (i == retry_attempts - 1)
            {
				THROW_HTTP(HttpException, "HTTP Error: " + networkErrorAsString(reply->error()) + "\nDevice Error: " + reply->errorString()+ "\nReply: " + output.body, output.status_code, output.headers, output.body);
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

ServerReply HttpRequestHandler::put(QString url, const QByteArray& data, const HttpHeaders& add_headers)
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
    ServerReply output;
    for (int i = 0; i < reply->rawHeaderList().size(); i++)
    {
        output.headers.insert(reply->rawHeaderList()[i], reply->rawHeader(reply->rawHeaderList()[i]));
    }
    output.status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    output.body = reply->readAll();

    if (reply->error()!=QNetworkReply::NoError)
    {
        THROW_HTTP(HttpException, "HTTP Error: " + networkErrorAsString(reply->error()) + "\nIODevice Error: " + reply->errorString(), output.status_code, output.headers, output.body);
	}
    reply->deleteLater();
    return output;
}

ServerReply HttpRequestHandler::post(QString url, const QByteArray& data, const HttpHeaders& add_headers)
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
    ServerReply output;
    for (int i = 0; i < reply->rawHeaderList().size(); i++)
    {
        output.headers.insert(reply->rawHeaderList()[i], reply->rawHeader(reply->rawHeaderList()[i]));
    }
    output.status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    output.body = reply->readAll();
    if (reply->error()!=QNetworkReply::NoError)
    {
		THROW_HTTP(HttpException, "HTTP Error: " + networkErrorAsString(reply->error()) + "\nDevice error: " + reply->errorString() + "\nReply: " + output.body, output.status_code, output.headers, output.body);
    }
    reply->deleteLater();
    return output;
}

ServerReply HttpRequestHandler::post(QString url, QHttpMultiPart* parts, const HttpHeaders& add_headers)
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
    ServerReply output;
    for (int i = 0; i < reply->rawHeaderList().size(); i++)
    {
        output.headers.insert(reply->rawHeaderList()[i], reply->rawHeader(reply->rawHeaderList()[i]));
    }
    output.status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    output.body = reply->readAll();
    if (reply->error()!=QNetworkReply::NoError)
    {
        THROW_HTTP(HttpException, "HTTP Error: " + networkErrorAsString(reply->error()) + "\nIODevice Error: " + reply->errorString(), output.status_code, output.headers, output.body);
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

QString HttpRequestHandler::networkErrorAsString(QNetworkReply::NetworkError error)
{

    switch(error)
    {
        case QNetworkReply::NetworkError::NoError: return "";

        // network layer errors [relating to the destination server] (1-99):
        case QNetworkReply::NetworkError::ConnectionRefusedError: return "Connection refused";
        case QNetworkReply::NetworkError::RemoteHostClosedError: return "Remote host closed the connection";
        case QNetworkReply::NetworkError::HostNotFoundError: return "Host not found";
        case QNetworkReply::NetworkError::TimeoutError: return "Connection timeout has been reached";
        case QNetworkReply::NetworkError::OperationCanceledError: return "Operation has been canceled";
        case QNetworkReply::NetworkError::SslHandshakeFailedError: return "SSL handshake failed";
        case QNetworkReply::NetworkError::TemporaryNetworkFailureError: return "Temporary network failure";
        case QNetworkReply::NetworkError::NetworkSessionFailedError: return "Network session failed";
        case QNetworkReply::NetworkError::BackgroundRequestNotAllowedError: return "Background request not allowed";
        case QNetworkReply::NetworkError::TooManyRedirectsError: return "Too many redirects";
        case QNetworkReply::NetworkError::InsecureRedirectError: return "Insecure redirect";
        case QNetworkReply::NetworkError::UnknownNetworkError: return "Unknown network error";

        // proxy errors (101-199):
        case QNetworkReply::NetworkError::ProxyConnectionRefusedError: return "Proxy connection has been refused";
        case QNetworkReply::NetworkError::ProxyConnectionClosedError: return "Proxy connection has been closed";
        case QNetworkReply::NetworkError::ProxyNotFoundError: return "Proxy not found";
        case QNetworkReply::NetworkError::ProxyTimeoutError: return "Proxy timeout";
        case QNetworkReply::NetworkError::ProxyAuthenticationRequiredError: return "Proxy authentication required";
        case QNetworkReply::NetworkError::UnknownProxyError: return "Unknown proxy error";

        // content errors (201-299):
        case QNetworkReply::NetworkError::ContentAccessDenied: return "Content access denied";
        case QNetworkReply::NetworkError::ContentOperationNotPermittedError: return "Content operation not permitted";
        case QNetworkReply::NetworkError::ContentNotFoundError: return "Content not found";
        case QNetworkReply::NetworkError::AuthenticationRequiredError: return "Authentication required";
        case QNetworkReply::NetworkError::ContentReSendError: return "Content ReSend";

        case QNetworkReply::NetworkError::ContentConflictError: return "Content conflict";
        case QNetworkReply::NetworkError::ContentGoneError: return "Content gone";
        case QNetworkReply::NetworkError::UnknownContentError: return "Unknown content error";

        // protocol errors
        case QNetworkReply::NetworkError::ProtocolUnknownError: return "Protocol unknown";
        case QNetworkReply::NetworkError::ProtocolInvalidOperationError: return "Protocol invalid operation";
        case QNetworkReply::NetworkError::ProtocolFailure: return "Protocol failure";

        // Server side errors (401-499)
        case QNetworkReply::NetworkError::InternalServerError: return "Internal server error";
        case QNetworkReply::NetworkError::OperationNotImplementedError: return "Operation not implemented";
        case QNetworkReply::NetworkError::ServiceUnavailableError: return "Service unavailable";
        case QNetworkReply::NetworkError::UnknownServerError: return "Unknown server error";
        default: return "Unknown network error";
    }
}

