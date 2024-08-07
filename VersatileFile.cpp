#include "VersatileFile.h"
#include "Helper.h"
#include <QUrl>
#include <QFileInfo>
#include <QDir>
#include <QThread>
#include <QTemporaryFile>
#include "CustomProxyService.h"

VersatileFile::VersatileFile(QString file_name)
	: file_name_(file_name)
	, cursor_position_(0)
{	
	if (isLocal())
	{
		local_source_ = QSharedPointer<QFile>(new QFile(file_name_));
	}
	else
	{
		socket_ = new QSslSocket();
		socket_->setSocketOption(QAbstractSocket::KeepAliveOption, 1);

        //set a proxy, if custom proxy settings have been provided
        if (CustomProxyService::getProxy() != QNetworkProxy::NoProxy)
        {
            socket_->setProxy(CustomProxyService::getProxy());
        }

		QUrl file_url(file_name_);
		server_path_ = file_url.path() + (file_url.hasQuery() ? "?" + file_url.query() : "");
		host_name_ = file_url.host();
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

	if (!socket_->isOpen()) socket_->open(mode);
	return socket_->isOpen();
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
	return socket_->openMode();
}

bool VersatileFile::isOpen() const
{
	if (isLocal()) return local_source_.data()->isOpen();
	return socket_->isOpen();
}

bool VersatileFile::isReadable() const
{
	if (isLocal())
	{
		if (QFileInfo(local_source_.data()->fileName()).isDir())
		{
			return QDir(local_source_.data()->fileName()).isReadable();
		}
		else
		{
			if (!local_source_->isOpen()) local_source_->open(QIODevice::ReadOnly);
			return local_source_.data()->isReadable();
		}
	}

	return file_size_ > 0;
}

QByteArray VersatileFile::read(qint64 maxlen)
{
	checkIfOpen();
	if (isLocal()) return local_source_.data()->read(maxlen);

	qint64 end = cursor_position_ + maxlen;
	if (end > file_size_) end = file_size_;

	QByteArray response = readResponseWithoutHeaders(createByteRangeRequestText(cursor_position_, end));
	cursor_position_ = cursor_position_ + response.length() - 1;
	if (cursor_position_ > file_size_) cursor_position_ = file_size_;
	return response;
}

QByteArray VersatileFile::readAll()
{
	checkIfOpen();
	if (isLocal()) return local_source_.data()->readAll();

	QByteArray response = readResponseWithoutHeaders(createGetRequestText());
	cursor_position_ = cursor_position_ + response.length();
	if (cursor_position_ > file_size_) cursor_position_ = file_size_;

	return response;
}

QByteArray VersatileFile::readLine(qint64 maxlen)
{
	checkIfOpen();
	if (isLocal()) return local_source_.data()->readLine(maxlen);

	if (readline_pointer_.isNull())
	{
		QTemporaryFile temp_file;
        if (!temp_file.open()) THROW(FileAccessException, "Could not initiate a temporary file for remote data!");
        QTemporaryFile temp_gz_file;
        if (!temp_gz_file.open()) THROW(FileAccessException, "Could not initiate a temporary file for compressed data!");

        QSharedPointer<QFile> buffer_file(new QFile(temp_file.fileName()));
        if (!buffer_file.data()->open(QIODevice::WriteOnly)) THROW(FileAccessException, "Could not open a temporary file for remote data: " + temp_file.fileName());
        buffer_file.data()->write(readResponseWithoutHeaders(createGetRequestText()));
        buffer_file.data()->close();

		file_size_ = QFileInfo(temp_file.fileName()).size();
        QString src_file = temp_file.fileName();

		// Special handling of *.vcf.gz files: they need to be unzipped
		if (QUrl(file_name_.toLower()).toString(QUrl::RemoveQuery).endsWith(".vcf.gz"))
		{
            gzFile gz_file = gzopen(temp_file.fileName().toUtf8(), "rb");
			if(!gz_file)
			{
				THROW(FileAccessException, "Could not open GZ file!");
			}

            QSharedPointer<QFile> gz_buffer_file(new QFile(temp_gz_file.fileName()));
            if (!gz_buffer_file.data()->open(QIODevice::WriteOnly|QIODevice::Append)) THROW(FileAccessException, "Could not open a temporary file for compressed data: " + temp_gz_file.fileName());

			const int buffer_size = 1048576; //1MB buffer
			char* gz_buffer = new char[buffer_size];
			while(int read_bytes =  gzread (gz_file, gz_buffer, buffer_size))
			{
                gz_buffer_file.data()->write(QByteArray(gz_buffer, read_bytes));
			}
			gzclose(gz_file);
			gz_buffer_file.data()->close();
            src_file = temp_gz_file.fileName();
		}
        readline_pointer_ = Helper::openFileForReading(src_file);
	}

	QByteArray line = readline_pointer_.data()->readLine(maxlen);
	cursor_position_ = readline_pointer_.data()->pos();

	return line;
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

	try
	{
		readAllViaSocket(createHeadRequestText());
		return true;
	}
	catch(Exception& e)
	{
		return false;
	}
}

void VersatileFile::close()
{
	checkIfOpen();
	if (isLocal())
	{
		local_source_.data()->close();
	}
	else
	{
		socket_->close();
//		worker_->exit(0);
	}
}

qint64 VersatileFile::pos() const
{
	checkIfOpen();
	if (isLocal()) return local_source_.data()->pos();
	return cursor_position_;
}

bool VersatileFile::seek(qint64 pos)
{
	checkIfOpen();
	if (isLocal()) return local_source_.data()->seek(pos);

	cursor_position_ = pos;
	if (cursor_position_ >= file_size_) cursor_position_ = file_size_;
	return cursor_position_ <= file_size_;
}

qint64 VersatileFile::size()
{
	if (isLocal()) return local_source_.data()->size();
	checkIfOpen();
	return getFileSize();
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
		if (!socket_->isOpen()) THROW(FileAccessException, "No connection to the remote file!");
	}
}

void VersatileFile::checkResponse(QByteArray& response) const
{
	QByteArray const http_version = "HTTP/1.1 ";
	if (response.isEmpty()) THROW(FileAccessException, "Empty response from the server!");
	int start_pos = response.toLower().indexOf(http_version.toLower());
	int response_code = response.mid(start_pos + http_version.length(), 3).toInt();
	if ((response_code != 200) && (response_code != 206)) THROW(FileAccessException, "Server replied with the code " + QString::number(response_code));
}

bool VersatileFile::isLocal() const
{
	return !Helper::isHttpUrl(file_name_);
}

bool VersatileFile::isEncrypted() const
{
	if (file_name_.startsWith("https://", Qt::CaseInsensitive)) return true;
	return false;
}

quint16 VersatileFile::getPortNumber()
{
	QUrl file_url(file_name_);
	int port = file_url.port();
	if (port > 0) return port;
	if (isEncrypted()) return 443;
	return 80;
}

void VersatileFile::addCommonHeaders(QByteArray& request)
{
	request.append(server_path_.toUtf8());
	request.append(" HTTP/1.1\r\n");
	request.append("Host: ");
	request.append(host_name_.toUtf8() + ":");
	request.append(QString::number(server_port_).toUtf8());
	request.append("\r\n");
	request.append("User-Agent: GSvar\r\n");
	request.append("X-Custom-User-Agent: GSvar\r\n");
}

QByteArray VersatileFile::createHeadRequestText()
{
	QByteArray payload;
	payload.append("HEAD ");
	addCommonHeaders(payload);
	payload.append("\r\n");
	return payload;
}

QByteArray VersatileFile::createGetRequestText()
{
	QByteArray payload;
	payload.append("GET ");
	addCommonHeaders(payload);
	payload.append("Connection: keep-alive\r\n");
	payload.append("\r\n");	
	return payload;
}

QByteArray VersatileFile::createByteRangeRequestText(qint64 start, qint64 end)
{
	QByteArray payload;
	payload.append("GET ");	
	addCommonHeaders(payload);
	payload.append("Connection: keep-alive\r\n");
	payload.append("Range: bytes=");
	payload.append(QString::number(start).toUtf8());
	payload.append("-");
	if (end > 0) payload.append(QString::number(end).toUtf8());
	payload.append("\r\n");
	payload.append("\r\n");
	return payload;
}

void VersatileFile::initiateRequest(const QByteArray& http_request)
{
	try
	{
		if (socket_->state() != QSslSocket::SocketState::ConnectedState)
		{
			if (isEncrypted())
			{
				socket_->connectToHostEncrypted(host_name_, server_port_);
				socket_->ignoreSslErrors();
			}
			else
			{
				socket_->connectToHost(host_name_, server_port_);
			}

			socket_->waitForConnected();
			if (isEncrypted()) socket_->waitForEncrypted();
			socket_->open(QIODevice::ReadWrite);
		}

		socket_->write(http_request);
		socket_->flush();
		if (socket_->state() != QSslSocket::SocketState::UnconnectedState) socket_->waitForBytesWritten();
	}
	catch (Exception& e)
	{
		THROW(FileAccessException, "There was an error while connecting to the server " + host_name_ + ": " + e.message());
	}
}

QByteArray VersatileFile::readAllViaSocket(const QByteArray& http_request)
{
	QByteArray response;
	try
	{
		initiateRequest(http_request);
		while(socket_->waitForReadyRead())
		{
			if(socket_->bytesAvailable())
			{
				response.append(socket_->readAll());
			}
		}
	}
	catch (Exception& e)
	{
		THROW(FileAccessException, "There was an error while reading a remote file: " + e.message());
	}
	checkResponse(response);

	return response;
}

QByteArray VersatileFile::readLineViaSocket(const QByteArray& http_request, qint64 maxlen)
{
	if (atEnd()) return "";

	try
	{
		if (socket_->state() != QSslSocket::SocketState::ConnectedState)
		{
			initiateRequest(http_request);
			bool found_headers = false;
			bool read_first_line = false;
			while(socket_->waitForReadyRead())
			{
				while(socket_->bytesAvailable())
				{
					QByteArray line = socket_->readLine(maxlen);
					if (!read_first_line)
					{
						checkResponse(line);
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

		while((socket_->waitForReadyRead()) || (socket_->bytesAvailable()))
		{
			if (socket_->canReadLine())
			{
				QByteArray result_line = socket_->readLine(maxlen);
				cursor_position_ = cursor_position_ + result_line.length();
				return result_line;
			}
		}
	}
	catch (Exception& e)
	{
		THROW(FileAccessException, "There was an error while reading a line from a remote file: " + e.message());
	}

	return "";
}

qint64 VersatileFile::getFileSize()
{
	if (exists())
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
	}

	return 0;
}

QByteArray VersatileFile::readResponseWithoutHeaders(const QByteArray &http_request)
{
	QByteArray response = readAllViaSocket(http_request);
	qint64 sep = response.indexOf("\r\n\r\n");
	response = response.mid(sep).replace(0, 4, "");
	return response;
}

