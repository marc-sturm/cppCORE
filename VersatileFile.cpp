#include "VersatileFile.h"
#include "Helper.h"
#include <QUrl>
#include <QFileInfo>
#include <QDir>
#include <QTemporaryFile>
#include "CustomProxyService.h"
#include "Log.h"

VersatileFile::VersatileFile(QString file_name)
	: proxy_(QNetworkProxy::NoProxy)
	, file_name_(file_name)
	, file_stream_pointer_(nullptr)
	, is_open_(false)
	, cursor_position_(0)
{
	//determine non-default mode
	if (Helper::isHttpUrl(file_name_))
	{
		mode_ = URL;
	}
	else if (file_name_.toLower().endsWith(".gz"))
	{
		mode_ = LOCAL_GZ;
	}

	//init members depending on mode
	if (mode_==LOCAL)
	{
		local_source_ = QSharedPointer<QFile>(new QFile(file_name_));
	}
	else if (mode_==LOCAL_GZ)
	{
		//norhing to do her - see open method
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

VersatileFile::VersatileFile(QString file_name, FILE* f)
	: proxy_(QNetworkProxy::NoProxy)
	, file_name_(file_name)
	, file_stream_pointer_(f)
	, cursor_position_(0)
{
	local_source_ = QSharedPointer<QFile>(new QFile(file_name_));
}

VersatileFile::~VersatileFile()
{
	close();
}

bool VersatileFile::open(QIODevice::OpenMode mode, bool throw_on_error)
{
	if (mode!=QFile::ReadOnly && mode!=(QFile::ReadOnly|QFile::Text))
	{
		THROW(ProgrammingException, "Invalid open mode '" + QString::number(mode) + "' in !");
	}

	bool opened = true;
	if (mode_==LOCAL)
	{
		if (file_stream_pointer_!=nullptr)
		{
			opened = local_source_.data()->open(file_stream_pointer_, mode);
		}
		else
		{
			opened = local_source_.data()->open(mode);
		}
	}
	else if (mode_==LOCAL_GZ)
	{
		QByteArray filename = file_name_.toUtf8();
		gz_stream_ = gzopen(filename.data(), "rb"); //read binary: always open in binary mode because Windows and Mac open in text mode
		if (!gz_stream_) opened = false;
		gz_buffer_ = new char[gz_buffer_size_];
		gzbuffer(gz_stream_, gz_buffer_size_internal_);
	}
	else
	{
		cursor_position_ = 0;
	}

	//throw exception if requested by user
	if (!opened && throw_on_error) THROW(FileAccessException, "Could not open file '" + file_name_ + "'");

	is_open_ = opened;

	return opened;
}

QIODevice::OpenMode VersatileFile::openMode() const
{
	if (mode_==LOCAL) return local_source_.data()->openMode();
	return QFile::ReadOnly;
}

void VersatileFile::setGzBufferSize(int bytes)
{
	if (isOpen()) THROW(ProgrammingException, "setGzBufferSize cannot be used after opening the file!");

	gz_buffer_size_ = bytes;
}

void VersatileFile::setGzBufferSizeInternal(int bytes)
{
	if (isOpen()) THROW(ProgrammingException, "setGzBufferSize cannot be used after opening the file!");

	gz_buffer_size_internal_ = bytes;
}

bool VersatileFile::isReadable() const
{
	if (mode_==LOCAL || mode_==LOCAL_GZ)
	{
		if (QFileInfo(file_name_).isDir())
		{
			return QDir(file_name_).isReadable();
		}
		else
		{
			QFile f(file_name_);
			if (!f.open(QFile::ReadOnly)) return false;
			return f.isReadable();
		}
	}

	return file_size_ > 0;
}

QByteArray VersatileFile::read(qint64 maxlen)
{
	if (!is_open_) THROW(ProgrammingException, QString(__FUNCTION__) + " called, on not open file '" + file_name_ + "!");

	if (mode_==LOCAL)
	{
		return local_source_.data()->read(maxlen);
	}
	else if (mode_==LOCAL_GZ)
	{
		THROW(NotImplementedException, "VersatileFile::read is not implemented for GZ files!");
	}

	qint64 end = cursor_position_ + maxlen;
	if (end > file_size_) end = file_size_;

    QByteArray response = sendByteRangeRequestText(cursor_position_, end).body;
	cursor_position_ = cursor_position_ + response.length() - 1;
	if (cursor_position_ > file_size_) cursor_position_ = file_size_;
	return response;
}

QByteArray VersatileFile::readAll()
{
	if (!is_open_) THROW(ProgrammingException, QString(__FUNCTION__) + " called, on not open file '" + file_name_ + "!");

	if (mode_==LOCAL)
	{
		return local_source_.data()->readAll();
	}
	else if (mode_==LOCAL_GZ)
	{
		QByteArray output;
		while (!atEnd())
		{
			output.append(readLine());
		}
		return output;
	}

    QByteArray response = sendGetRequestText().body;
	cursor_position_ = cursor_position_ + response.length();
	if (cursor_position_ > file_size_) cursor_position_ = file_size_;

	return response;
}

QByteArray VersatileFile::readLine(bool trim_line_endings)
{
	if (!is_open_) THROW(ProgrammingException, QString(__FUNCTION__) + " called, on not open file '" + file_name_ + "!");

	QByteArray output;

	if (mode_==LOCAL)
	{
		output = local_source_.data()->readLine();
	}
	else if (mode_==LOCAL_GZ)
	{
		// get next line
		char* char_array = gzgets(gz_stream_, gz_buffer_, gz_buffer_size_);

		//handle errors like truncated GZ file
		if (char_array==nullptr)
		{
			int error_no = Z_OK;
			QByteArray error_message = gzerror(gz_stream_, &error_no);
			if (error_no!=Z_OK && error_no!=Z_STREAM_END)
			{
				THROW(FileParseException, "Error while reading file '" + file_name_ + "': " + error_message);
			}
		}

		output = QByteArray(char_array);
	}
	else
	{
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

			//.gz files need to be unzipped
			if (QUrl(file_name_.toLower()).toString(QUrl::RemoveQuery).endsWith(".gz"))
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

		output = readline_pointer_.data()->readLine();
		cursor_position_ = readline_pointer_.data()->pos();
	}

	while (trim_line_endings && (output.endsWith('\n') || output.endsWith('\r')))
	{
		output.chop(1);
	}

	return output;
}

bool VersatileFile::atEnd() const
{
	if (!is_open_) THROW(ProgrammingException, QString(__FUNCTION__) + " called, on not open file '" + file_name_ + "!");

	if (mode_==LOCAL)
	{
		return local_source_.data()->atEnd();
	}
	else if (mode_==LOCAL_GZ)
	{
		return gzeof(gz_stream_);
	}

	return (cursor_position_ >= file_size_);
}

bool VersatileFile::exists()
{
	if (mode_==LOCAL || mode_==LOCAL_GZ)
	{
		return QFile(file_name_).exists();
	}

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
	if (mode_==LOCAL)
	{
		local_source_.data()->close();
	}
	else if (mode_==LOCAL_GZ)
	{
		if (gz_stream_!=nullptr)
		{
			gzclose(gz_stream_);
			gz_stream_ = nullptr;
		}

		if (gz_buffer_!=nullptr)
		{
			delete gz_buffer_;
			gz_buffer_ = nullptr;
		}
	}

	is_open_ = false;
}

qint64 VersatileFile::pos() const
{
	if (!is_open_) THROW(ProgrammingException, QString(__FUNCTION__) + " called, on not open file '" + file_name_ + "!");

	if (mode_==LOCAL)
	{
		return local_source_.data()->pos();
	}
	else if (mode_==LOCAL_GZ)
	{
		THROW(NotImplementedException, "VersatileFile::pos is not implemented for GZ files!");
	}

	return cursor_position_;
}

bool VersatileFile::seek(qint64 pos)
{
	if (!is_open_) THROW(ProgrammingException, QString(__FUNCTION__) + " called, on not open file '" + file_name_ + "!");

	if (mode_==LOCAL)
	{
		return local_source_.data()->seek(pos);
	}
	else if (mode_==LOCAL_GZ)
	{
		THROW(NotImplementedException, "VersatileFile::seek is not implemented for GZ files!");
	}

	cursor_position_ = pos;
	if (cursor_position_ >= file_size_) cursor_position_ = file_size_;
	return cursor_position_ <= file_size_;
}

qint64 VersatileFile::size()
{
    //if (!is_open_) THROW(ProgrammingException, QString(__FUNCTION__) + " called, on not open file '" + file_name_ + "!");

	if (mode_==LOCAL)
	{
		return local_source_.data()->size();
	}
	else if (mode_==LOCAL_GZ)
	{
		THROW(NotImplementedException, "VersatileFile::size is not implemented for GZ files!");
	}

	return file_size_;
}

QString VersatileFile::fileName() const
{
	return file_name_;
}

void VersatileFile::checkResponse(QByteArray& response) const
{
	QByteArray const http_version = "HTTP/1.1 ";
	if (response.isEmpty()) THROW(FileAccessException, "Empty response from the server!");
	int start_pos = response.toLower().indexOf(http_version.toLower());
	int response_code = response.mid(start_pos + http_version.length(), 3).toInt();
	if ((response_code != 200) && (response_code != 206)) THROW(FileAccessException, "Server replied with the code " + QString::number(response_code));
}

void VersatileFile::addCommonHeaders(HttpHeaders& request_headers)
{
    request_headers.insert("User-Agent", "GSvar");
    request_headers.insert("X-Custom-User-Agent", "GSvar");
}

ServerReply VersatileFile::sendHeadRequest()
{
    HttpHeaders add_headers;
    addCommonHeaders(add_headers);
    return HttpRequestHandler(proxy_).head(file_name_, add_headers);
}

ServerReply VersatileFile::sendGetRequestText()
{	
    HttpHeaders add_headers;
    addCommonHeaders(add_headers);
    add_headers.insert("Connection", "keep-alive");
    return HttpRequestHandler(proxy_).get(file_name_, add_headers);
}

ServerReply VersatileFile::sendByteRangeRequestText(qint64 start, qint64 end)
{
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

qint64 VersatileFile::getFileSize()
{
	if (exists())
	{        
        ServerReply reply = sendHeadRequest();
        foreach(const QByteArray& item, reply.headers.keys())
        {
            if (item.toLower() == "content-length")
            {
                return reply.headers[item].toLongLong();
            }
        }
	}

	return 0;
}

