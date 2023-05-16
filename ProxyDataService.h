#ifndef PROXYDATASERVICE_H
#define PROXYDATASERVICE_H

#include "cppCORE_global.h"
#include <QNetworkProxy>
#include <QObject>


class CPPCORESHARED_EXPORT ProxyDataService
		: public QObject
{
	Q_OBJECT
public:
	ProxyDataService();
	bool setCredentials(QString user, QString password);

signals:
	void proxyAuthenticationRequired();

private:
	QNetworkProxy proxy_;

	void initProxy();
	static bool test_connection(const QNetworkProxy& proxy);

};

#endif // PROXYDATASERVICE_H
