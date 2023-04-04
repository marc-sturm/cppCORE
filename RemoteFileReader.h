#ifndef REMOTEFILEREADER_H
#define REMOTEFILEREADER_H

#include <QObject>
#include <QByteArray>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

class RemoteFileReader : public QObject
{
	Q_OBJECT
public:
	explicit RemoteFileReader(QUrl file_url, QObject *parent = 0);
	virtual ~RemoteFileReader();
	void start();
	QByteArray downloadedData() const;

signals:
	void downloaded();

private slots:
	void fileDownloaded(QNetworkReply* reply);

private:
	QUrl file_url_;
	QNetworkAccessManager network_manager_;
	QByteArray file_content_;
};

#endif // REMOTEFILEREADER_H
