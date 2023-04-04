#include "RemoteFileReader.h"

RemoteFileReader::RemoteFileReader(QUrl file_url, QObject *parent) :
	file_url_(file_url)
  , QObject(parent)
{
	connect(&network_manager_, SIGNAL (finished(QNetworkReply*)), this, SLOT (fileDownloaded(QNetworkReply*)));
}

RemoteFileReader::~RemoteFileReader()
{
}

void RemoteFileReader::start()
{
	QNetworkRequest request(file_url_);
	network_manager_.get(request);
}

void RemoteFileReader::fileDownloaded(QNetworkReply* reply)
{
	file_content_ = reply->readAll();
	reply->deleteLater();
	emit downloaded();
}

QByteArray RemoteFileReader::downloadedData() const
{
	return file_content_;
}
