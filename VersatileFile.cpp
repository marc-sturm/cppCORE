#include "VersatileFile.h"
#include "Helper.h"
#include <QUrl>
#include <QFileInfo>
#include <QDir>
#include <QTemporaryFile>
#include "CustomProxyService.h"
#include "Log.h"

VersatileFile::VersatileFile(QString file_name)
	: file_name_(file_name)
	, cursor_position_(0)
    , proxy_(QNetworkProxy::NoProxy)
{
    Log::info("Requesting file: " + file_name_);
    if (isLocal())
	{
		local_source_ = QSharedPointer<QFile>(new QFile(file_name_));
	}
	else
    {
        //set a proxy, if custom proxy settings have been provided
        if (CustomProxyService::getProxy() != QNetworkProxy::NoProxy)
        {
            proxy_ = CustomProxyService::getProxy();
        }

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

    return true;
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
    return QFile::ReadWrite;
}

bool VersatileFile::isOpen() const
{
	if (isLocal()) return local_source_.data()->isOpen();
    return true;
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

    QByteArray response = sendByteRangeRequestText(cursor_position_, end).body;
	cursor_position_ = cursor_position_ + response.length() - 1;
	if (cursor_position_ > file_size_) cursor_position_ = file_size_;
	return response;
}

QByteArray VersatileFile::readAll()
{
	checkIfOpen();
	if (isLocal()) return local_source_.data()->readAll();

    QByteArray response = sendGetRequestText().body;
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
        buffer_file.data()->write(sendGetRequestText().body);
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
        ServerReply reply = sendHeadRequest();
        return reply.status_code != 404;
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

void VersatileFile::addCommonHeaders(HttpHeaders& request_headers)
{
    request_headers.insert("User-Agent", "GSvar");
    request_headers.insert("X-Custom-User-Agent", "GSvar");
}

ServerReply VersatileFile::sendHeadRequest()
{
    try
    {
        Log::info("HEAD request for " + file_name_);
        HttpHeaders add_headers;
        addCommonHeaders(add_headers);
        return HttpRequestHandler(proxy_).head(file_name_, add_headers);
    }
    catch (HttpException& e)
    {
        Log::error("An error while performing a HEAD request for the file '" + file_name_ + "': " + e.message());

    }
    return ServerReply();
}

ServerReply VersatileFile::sendGetRequestText()
{	
    try
    {
        Log::info("GET request for " + file_name_);
        HttpHeaders add_headers;
        addCommonHeaders(add_headers);
        add_headers.insert("Connection", "keep-alive");
        return HttpRequestHandler(proxy_).get(file_name_, add_headers);
    }
    catch (HttpException& e)
    {
        Log::error("An error while performing a GET request for the file '" + file_name_ + "': " + e.message());
    }
    return ServerReply();
}

ServerReply VersatileFile::sendByteRangeRequestText(qint64 start, qint64 end)
{
    try
    {
        Log::info("GET Range request for " + file_name_);
        HttpHeaders add_headers;
        add_headers.insert("Connection", "keep-alive");
        addCommonHeaders(add_headers);

        QByteArray range_values = "bytes=" + QString::number(start).toUtf8() + "-";
        if (end>0)
        {
            range_values.append(QString::number(end).toUtf8());
        }
        add_headers.insert("Range", range_values);
        return HttpRequestHandler(proxy_).get(file_name_, add_headers);
    }
    catch (HttpException& e)
    {
        Log::error("An error while performing a GET request for the file '" + file_name_ + "': " + e.message());
    }
    return ServerReply();
}

qint64 VersatileFile::getFileSize()
{
	if (exists())
	{        
        ServerReply reply = sendHeadRequest();
        foreach(const QByteArray& item, reply.headers.keys())
        {
            if (item.toLower() == "content-length")
            {
                Log::info("Size from Network Manager: " + reply.headers[item]);
                return reply.headers[item].toLongLong();
            }
        }
	}

	return 0;
}

