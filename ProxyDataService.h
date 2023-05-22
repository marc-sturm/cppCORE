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
	static bool setCredentials(QString user, QString password);
	static const QNetworkProxy& getProxy();
	static bool isConnected();

private:
	QNetworkProxy proxy_;

	ProxyDataService();
	static ProxyDataService& instance();
	static bool test_connection(QNetworkProxy proxy);

};

#endif // PROXYDATASERVICE_H
