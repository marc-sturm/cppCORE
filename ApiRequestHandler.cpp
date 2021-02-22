#include "ApiRequestHandler.h"
#include "HttpRequestHandler.h"

ApiRequestHandler::ApiRequestHandler(const QString server_host, const int server_port)
	: server_host_(server_host)
	, server_port_(server_port)
{
}

ApiRequestHandler::~ApiRequestHandler()
{
}

QString ApiRequestHandler::sendGetRequest(const QString path)
{
	QString reply {};

	if (server_host_.isEmpty())
	{
		THROW(ArgumentException, "Server host has not been provided")
		return reply;
	}

	if (server_port_ == 0)
	{
		THROW(ArgumentException, "Server port has not been provided")
		return reply;
	}

	HttpHeaders add_headers;
	add_headers.insert("Accept", "application/json");
	reply = HttpRequestHandler(HttpRequestHandler::NONE).get("https://" + server_host_ + ":" + QString::number(server_port_) + path, add_headers);

	return reply;
}
