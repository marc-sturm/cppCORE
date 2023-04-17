#include "RemoteReader.h"

RemoteReader::RemoteReader(QString& src, QByteArray& out, qint64 size)
	: QRunnable()
	, terminate_(false)
	, src_(src)
	, out_(out)
	, size_(size)
{

}

void RemoteReader::run()
{

	QUrl file_url(src_);
	QString server_path = file_url.path() + (file_url.hasQuery() ? "?" + file_url.query() : "");
	QString host_name = file_url.host();
	int server_port = file_url.port();
	if (server_port<=0)
	{
		if (src_.startsWith("https", Qt::CaseInsensitive)) server_port = 443;
		//if (src_.startsWith("http", Qt::CaseInsensitive)) server_port = 80;
	}

	QByteArray request;
	request.append("GET ");
	request.append(server_path.toUtf8());
	request.append(" HTTP/1.1\r\n");
	request.append("Host: ");
	request.append(host_name.toUtf8() + ":");
	request.append(QString::number(server_port).toUtf8());
	request.append("\r\n");
	request.append("User-Agent: GSvar\r\n");
	request.append("X-Custom-User-Agent: GSvar\r\n");
	request.append("Connection: keep-alive\r\n");
	request.append("\r\n");


	qDebug() << "server_port" << server_port;






//	QByteArray response;
	QSslSocket *socket_ = new QSslSocket();
	socket_->setSocketOption(QAbstractSocket::KeepAliveOption, 1);
//	try
//	{

//		try
//		{
			if (socket_->state() != QSslSocket::SocketState::ConnectedState)
			{
				if (src_.startsWith("https://", Qt::CaseInsensitive))
				{
					socket_->connectToHostEncrypted(host_name, server_port);
					socket_->ignoreSslErrors();
				}
				else
				{
					socket_->connectToHost(host_name, server_port);
				}

				qDebug() << "Connected...";
				socket_->waitForConnected();
				if (src_.startsWith("https://", Qt::CaseInsensitive)) socket_->waitForEncrypted();
				socket_->open(QIODevice::ReadWrite);
			}

			socket_->write(request);
			socket_->flush();
			if (socket_->state() != QSslSocket::SocketState::UnconnectedState) socket_->waitForBytesWritten();

			qDebug() << socket_->state();

			//		}
//		catch (Exception& e)
//		{
//			THROW(FileAccessException, "There was an error while connecting to the server " + host_name_ + ": " + e.message());
//		}

		qDebug() << "Prepare to read...";

		bool found_headers = false;
		while(socket_->waitForReadyRead())
		{
			//qDebug() << "Ready to read";
			if(socket_->bytesAvailable())
			{
				//qDebug() << "Reading";
				QByteArray part = socket_->readAll();
				//qDebug() << part.size();
				out_.append(part);
				if (!found_headers)
				{
					qint64 sep = out_.indexOf("\r\n\r\n");
					if (sep>-1)
					{
						out_ = out_.mid(sep).replace(0, 4, "");
						found_headers = true;
					}
				}
				if (out_.size()>=size_) break;

			}
		}




		//qDebug() << "out_" << out_;
//	}
//	catch (Exception& e)
//	{
//		THROW(FileAccessException, "There was an error while reading a remote file: " + e.message());
//	}




//	while(out_.size()<content_length.toLongLong())
//	{
//		if (!response->bytesAvailable()) response->waitForReadyRead(10000);
//		QByteArray data_part = response->readAll();
//		qDebug() << data_part;
//		out_.append(data_part);

//	}

	//	QString html = response->readAll();
}


