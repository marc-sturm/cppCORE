#ifndef APIREQUESTHANDLER_H
#define APIREQUESTHANDLER_H

#include "cppCORE_global.h"
#include "Exceptions.h"

class CPPCORESHARED_EXPORT ApiRequestHandler
{
public:
	ApiRequestHandler(const QString server_host, const int server_port);
	~ApiRequestHandler();
	QString sendGetRequest(const QString path);

protected:
	QString server_host_;
	int server_port_;
};

#endif // APIREQUESTHANDLER_H
