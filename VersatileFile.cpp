#include "VersatileFile.h"

VersatileFile::VersatileFile(const QString& file_name)
	: file_name_(file_name)
{
	if (!isLocal())
	{
		socket_ = new QSslSocket();
		socket_->setSocketOption(QAbstractSocket::KeepAliveOption, 1);

		server_path_ = getServerPath();
		host_name_ = getHostName();
		server_port_ = getPortNumber();
		file_size_ = getFileSize();
	}
}

VersatileFile::~VersatileFile()
{
}

bool VersatileFile::open(QIODevice::OpenMode mode)
{
	if (isLocal())
	{
		local_source_ = QSharedPointer<QFile>(new QFile(file_name_));
		local_source_.data()->open(mode);
		return local_source_.data()->isOpen();
	}

	cursor_position_ = 0;
	remote_source_ = QSharedPointer<QSslSocket>(socket_);
	remote_source_.data()->open(mode);
	return remote_source_.data()->isOpen();
}

bool VersatileFile::open(FILE* f, QIODevice::OpenMode ioFlags)
{
	if (isLocal())
	{
		local_source_ = QSharedPointer<QFile>(new QFile(file_name_));
		local_source_.data()->open(f, ioFlags);
		return local_source_.data()->isOpen();
	}
	return false;
}

QIODevice::OpenMode VersatileFile::openMode() const
{
	if (isLocal()) return local_source_.data()->openMode();
	return remote_source_.data()->openMode();
}

bool VersatileFile::isOpen() const
{
	if (isLocal()) return local_source_.data()->isOpen();
	return remote_source_.data()->isOpen();
}

bool VersatileFile::isReadable() const
{
	checkIfOpen();
	if (isLocal()) return local_source_.data()->isReadable();
	return remote_source_.data()->isReadable();
}

QByteArray VersatileFile::readAll()
{
	checkIfOpen();
	if (isLocal()) return local_source_.data()->readAll();
	return readResponseWithoutHeaders();
}

QByteArray VersatileFile::readLine(qint64 maxlen)
{
	checkIfOpen();
	if (isLocal()) return local_source_.data()->readLine(maxlen);
	return readLineWithoutHeaders();
}

bool VersatileFile::atEnd() const
{
	checkIfOpen();
	if (isLocal()) return local_source_.data()->atEnd();
	return (cursor_position_ >= file_size_);
}

bool VersatileFile::exists()
{
	if (!local_source_.isNull()) return local_source_.data()->exists();
	return (file_size_ > 0);
}

void VersatileFile::close()
{
	if (!local_source_.isNull())
	{
		if (local_source_.data()->isOpen()) local_source_.data()->close();
	}
	if (!remote_source_.isNull())
	{
		if (remote_source_.data()->isOpen()) remote_source_.data()->close();
	}
}

qint64 VersatileFile::pos() const
{
	checkIfOpen();
	if (isLocal()) return local_source_.data()->pos();
	return cursor_position_;
}

qint64 VersatileFile::size() const
{
	checkIfOpen();
	if (isLocal()) return local_source_.data()->size();
	return file_size_;
}

QString VersatileFile::fileName() const
{
	return file_name_;
}

void VersatileFile::checkIfOpen() const
{
	if (isLocal())
	{
		if (local_source_.isNull()) THROW(FileAccessException, "Local file is not set!");
		if (!local_source_.data()->isOpen()) THROW(FileAccessException, "Local file is not open!");
	}
	else
	{
		if (remote_source_.isNull()) THROW(FileAccessException, "Remote file is not set!");
		if (!remote_source_.data()->isOpen()) THROW(FileAccessException, "Remote file is not open!");
	}
}

bool VersatileFile::isLocal() const
{
	return (!file_name_.startsWith("http", Qt::CaseInsensitive));
}

QString VersatileFile::getServerPath()
{
	QString server_path = file_name_;
	server_path = server_path.replace("https://", "", Qt::CaseInsensitive);
	QList<QString> parts = server_path.split("/");
	if (parts.size() < 2) server_path = server_path.replace(parts.takeFirst(), "", Qt::CaseInsensitive);
	return server_path;
}

QString VersatileFile::getHostName()
{
	QString host_name = file_name_;
	host_name = host_name.replace("https://", "", Qt::CaseInsensitive);
	QList<QString> url_parts = host_name.split("/");
	if (url_parts.size() > 1) host_name = url_parts.takeFirst();
	QList<QString> host_name_parts = host_name.split(":");
	if (host_name_parts.size()>1) host_name = host_name_parts.takeFirst();
	return host_name;
}

quint16 VersatileFile::getPortNumber()
{
	qint16 port_number = 443; // default HTTPS port
	QString host_name = file_name_;
	host_name = host_name.replace("https://", "", Qt::CaseInsensitive);
	QList<QString> url_parts = host_name.split("/");
	if (url_parts.size() > 1) host_name = url_parts.takeFirst();
	QList<QString> host_name_parts = host_name.split(":");
	if (host_name_parts.size()>1) port_number = host_name_parts.takeLast().toInt();
	return port_number;
}

QByteArray VersatileFile::createHeadRequestText()
{
	QByteArray payload;
	payload.append("HEAD ");
	payload.append(server_path_);
	payload.append(" HTTP/1.1\r\n");
	payload.append("Host: ");
	payload.append(host_name_ + ":");
	payload.append(server_port_);
	payload.append("\r\n");
	payload.append("Connection: keep-alive\r\n");
	payload.append("\r\n");
	return payload;
}

QByteArray VersatileFile::createGetRequestText()
{
	QByteArray payload;
	payload.append("GET ");
	payload.append(server_path_);
	payload.append(" HTTP/1.1\r\n");
	payload.append("Host: ");
	payload.append(host_name_ + ":");
	payload.append(server_port_);
	payload.append("\r\n");
	payload.append("Connection: keep-alive\r\n");
	payload.append("\r\n");
	return payload;
}

void VersatileFile::initiateRequest(const QByteArray& http_request)
{
	socket_->connectToHostEncrypted(host_name_, server_port_);
	socket_->ignoreSslErrors();
	socket_->waitForConnected();
	socket_->waitForEncrypted();

	socket_->open(QIODevice::ReadWrite);

	socket_->write(http_request);
	socket_->flush();
	socket_->waitForBytesWritten();
}

QByteArray VersatileFile::readAllViaSocket(const QByteArray& http_request)
{
	if (socket_->state() != QSslSocket::SocketState::ConnectedState)
	{
		initiateRequest(http_request);
	}

	QByteArray response;
	while(socket_->waitForReadyRead())
	{
		while(socket_->bytesAvailable())
		{
			response.append(socket_->readAll());
			cursor_position_ = response.length();
		}
	}

	return response;
}

QByteArray VersatileFile::readLineViaSocket(const QByteArray& http_request, qint64 maxlen)
{
	if (this->atEnd()) return "";

	if (buffer_.size() > 0)
	{
		QByteArray result_line = buffer_.first();
		buffer_.removeFirst();
		cursor_position_ = cursor_position_ + result_line.length();
		return result_line;
	}

	if (socket_->state() != QSslSocket::SocketState::ConnectedState)
	{
		initiateRequest(http_request);
		bool found_headers = false;
		while(socket_->waitForReadyRead())
		{
			while(socket_->bytesAvailable())
			{
				QString line = socket_->readLine(maxlen);
				if (line.trimmed().length() == 0)
				{
					found_headers = true;
					break;
				}
			}
			if (found_headers) break;
		}
	}

	QByteArray result_line;
	bool found_line = false;

	while((socket_->waitForReadyRead()) || (socket_->bytesAvailable()))
	{
		if (socket_->canReadLine())
		{
			found_line = true;
			result_line = socket_->readLine(maxlen);
			cursor_position_ = cursor_position_ + result_line.length();
		}
		if(socket_->bytesAvailable())
		{
			while (socket_->canReadLine())
			{
				buffer_.append(socket_->readLine(maxlen));
			}
		}
		if (found_line)
		{
			return result_line;
		}
	}

	return "";
}

qint64 VersatileFile::getFileSize()
{
	QByteArray response = readAllViaSocket(createHeadRequestText());
	cursor_position_ = 0;
	QTextStream stream(response);
	while(!stream.atEnd())
	{
		QString line = stream.readLine();
		if (line.startsWith("Content-Length", Qt::CaseInsensitive))
		{
			QList<QString> parts = line.split(":");
			if (parts.size() > 1) return parts.takeLast().trimmed().toLongLong();
		}
	}

	return 0;
}

QByteArray VersatileFile::readResponseWithoutHeaders()
{
	QByteArray response = readAllViaSocket(createGetRequestText());
	QTextStream stream(response);

	qint64 pos = 0;
	while(!stream.atEnd())
	{
		QString line = stream.readLine();
		pos = pos + line.length() ; // end of line characters (\r\n) separate headers from the body
		if (line.length() == 0)
		{
			break;
		}
	}
	return response.mid(pos);
}

QByteArray VersatileFile::readLineWithoutHeaders(qint64 maxlen)
{
	QByteArray response = readLineViaSocket(createGetRequestText(), maxlen);
	return response;
}

