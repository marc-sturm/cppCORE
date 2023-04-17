#ifndef REMOTEREADER_H
#define REMOTEREADER_H

#include <QString>
#include <QRunnable>
#include <QNetworkAccessManager>
#include <QUrl>
#include <QEventLoop>
#include <QNetworkReply>
//#include <QObject>

class RemoteReader
	: public QRunnable
{

public:
	RemoteReader(QString& src, QByteArray& out, qint64 size);
	void run();
	void terminate()
	{
		terminate_ = true;
	}

protected:
	bool terminate_;
	QString src_;
	QByteArray& out_;
	qint64 size_;
};


#endif // REMOTEREADER_H
