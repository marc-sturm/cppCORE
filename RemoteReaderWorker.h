#ifndef REMOTEREADERWORKER_H
#define REMOTEREADERWORKER_H

#include <QThread>
#include <QSslSocket>
#include <QSslError>
#include <QSslConfiguration>
#include <QList>
#include "Log.h"
#include "Exceptions.h"

class RemoteReaderWorker : public QThread
{
	Q_OBJECT
public:
	RemoteReaderWorker(QStringList& buffer, QString hostname, int port, QString path, bool& finished_reading);
	void run();

signals:
	void lineRetrieved(QString line);
	void reachedEnd();


private:
	QString hostname_;
	int port_;
	QString path_;
	QStringList& buffer_;
	bool& finished_reading_;
};

#endif // REMOTEREADERWORKER_H
