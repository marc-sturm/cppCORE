#include "RemoteReaderWorker.h"

RemoteReaderWorker::RemoteReaderWorker(QStringList& buffer, QString hostname, int port, QString path, bool& finished_reading)
	: buffer_(buffer)
	, hostname_(hostname)
	, port_(port)
	, path_(path)
	, finished_reading_(finished_reading)
{
}

void RemoteReaderWorker::run()
{
	finished_reading_ = false;
	QSslSocket* socket = new QSslSocket();
	socket->setSocketOption(QAbstractSocket::KeepAliveOption, 1);

	QByteArray request;
	request.append("GET ");
	request.append(path_.toUtf8());
	request.append(" HTTP/1.1\r\n");
	request.append("Host: ");
	request.append(hostname_.toUtf8() + ":");
	request.append(QString::number(port_).toUtf8());
	request.append("\r\n");
	request.append("User-Agent: GSvar\r\n");
	request.append("X-Custom-User-Agent: GSvar\r\n");
	request.append("Connection: keep-alive\r\n");
	request.append("\r\n");

	try
	{
//		if (file_name_.startsWith("https://", Qt::CaseInsensitive))
//		{
			socket->connectToHostEncrypted(hostname_, port_);
			socket->ignoreSslErrors();
//		}
//		else
//		{
//			socket_->connectToHost(hostname_, port_);
//		}

		socket->waitForConnected();
//		if (isEncrypted())
		socket->waitForEncrypted();
		socket->open(QIODevice::ReadWrite);
		socket->write(request);
		socket->flush();
		if (socket->state() != QSslSocket::SocketState::UnconnectedState) socket->waitForBytesWritten();
	}
	catch (Exception& e)
	{
		Log::error("There was an error while connecting to the server " + hostname_ + ": " + e.message());
		return;
	}

	if (socket->state() != QSslSocket::SocketState::ConnectedState)
	{
		bool found_headers = false;
		bool read_first_line = false;
		while(socket->waitForReadyRead())
		{
			while(socket->bytesAvailable())
			{
				QByteArray line = socket->readLine();
				if (!read_first_line)
				{
					read_first_line = true;
				}

				if (line.trimmed().length() == 0)
				{
					found_headers = true;
					break;
				}
			}
			if (found_headers) break;
		}
	}

	while((socket->waitForReadyRead()) || (socket->bytesAvailable()))
	{
		if (socket->canReadLine())
		{
			QString line = socket->readLine();
			buffer_.append(line);
//			emit lineRetrieved(line);
		}
	}
	finished_reading_ = true;
//	emit reachedEnd();
}
