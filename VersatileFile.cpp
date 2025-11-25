#include "VersatileFile.h"
#include "Helper.h"
#include <QUrl>
#include <QFileInfo>
#include <QDir>
#include "CustomProxyService.h"

VersatileFile::VersatileFile(QString file_name, bool stdin_if_empty)
	: proxy_(QNetworkProxy::NoProxy)
	, file_name_(file_name)
	, file_stream_pointer_(nullptr)
	, is_open_(false)
	, cursor_position_(0)
{
	//determine non-default mode
	if (Helper::isHttpUrl(file_name_))
	{
		mode_ = QUrl(file_name_).path().toLower().endsWith(".gz") ? URL_GZ : URL;
	}
	else
	{
		mode_ = file_name_.toLower().endsWith(".gz") ? LOCAL_GZ : LOCAL;
	}

	//init members depending on mode
	if (mode_==LOCAL)
	{
		local_source_ = QSharedPointer<QFile>(new QFile(file_name_));
		if (file_name=="" && stdin_if_empty)
		{
			file_stream_pointer_ = stdin;
		}
	}
	else if (mode_==LOCAL_GZ)
	{
        //nothing to do here - see open method
	}
	else
    {
        //set a proxy, if custom proxy settings have been provided
        if (CustomProxyService::getProxy() != QNetworkProxy::NoProxy)
        {
            proxy_ = CustomProxyService::getProxy();
            net_mgr_.setProxy(proxy_);
        }
        checkRemoteFile();
	}
}

VersatileFile::~VersatileFile()
{
	close();
}

bool VersatileFile::open(QIODevice::OpenMode mode, bool throw_on_error)
{
	if (mode!=QFile::ReadOnly && mode!=(QFile::ReadOnly|QFile::Text))
	{
		THROW(ProgrammingException, "Invalid open mode '" + QString::number(mode) + "' in VersatileFile::open(mode)!");
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
		gz_stream_ = gzopen(file_name_.toUtf8().data(), "rb"); //read binary: always open in binary mode because Windows and Mac open in text mode
		if (!gz_stream_) opened = false;
        gz_buffer_ = new char[gz_buffer_size_];
		gzbuffer(gz_stream_, gz_buffer_size_internal_);
	}
	else
	{
		remote_position_ = 0;
		cursor_position_ = 0;
        buffer_.clear();
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

    // regular remote file (URL mode)
    QByteArray result;
    while (result.size() < maxlen)
    {
		qint64 to_read = qMin(chunkSize(), maxlen - result.size());
		qint64 end_pos = qMin(cursor_position_ + to_read - 1, file_size_ - 1);
		QByteArray chunk = httpRangeRequest(cursor_position_, end_pos);
        if (chunk.isEmpty()) break;

        result.append(chunk);
        cursor_position_ += chunk.size();
		remote_position_ += chunk.size();
    }

    // remote file is a compressed file
    if (mode_==URL_GZ)
    {
        return decompressGzip(result);
    }

    return result;
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

    // regular remote file (URL mode)
    QNetworkRequest request((QUrl(file_name_)));
    QNetworkReply* reply = net_mgr_.get(request);

    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    QByteArray data;
    if (reply->error() == QNetworkReply::NoError)
    {
        data = reply->readAll();
        cursor_position_ += data.size();
    }
    reply->deleteLater();

    // remote file is a compressed file
    if (mode_==URL_GZ)
    {
        remote_gz_finished_ = true;
        QByteArray output = decompressGzip(data);
        decompressed_buffer_pos_ = output.size();
        return output;
    }

    return data;
}

QByteArray VersatileFile::readLine(bool trim_line_endings)
{
    int maxlen = 0; // temporary fix

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
	else if (mode_==URL_GZ)
	{
		while (true)
		{
			int newline_index = decompressed_buffer_.indexOf('\n', decompressed_buffer_pos_);
			if (newline_index != -1)
			{
				int line_length = newline_index - decompressed_buffer_pos_ + 1;
				if (maxlen > 0) line_length = qMin(line_length, (int)maxlen);

				output = decompressed_buffer_.mid(decompressed_buffer_pos_, line_length);                
				decompressed_buffer_pos_ += line_length;
				cursor_position_ += output.size();
				break;
			}

			if (remote_gz_finished_)
			{
				if (decompressed_buffer_pos_ >= decompressed_buffer_.size()) return QByteArray();

				output = decompressed_buffer_.mid(decompressed_buffer_pos_);
				cursor_position_ += output.size();
				decompressed_buffer_pos_ = decompressed_buffer_.size();
				break;
			}

			qint64 end = qMin(remote_position_ + chunkSize() - 1, size() - 1);			
			QByteArray compressed_chunk = httpRangeRequest(remote_position_, end);
			if (compressed_chunk.isEmpty())
			{
				remote_gz_finished_ = true;
				continue;
			}

			remote_position_ += compressed_chunk.size();          
            decompressed_buffer_.append(decompressGzip(compressed_chunk));
		}
	}
	else
	{
		// Regular remote text file
		while (true)
		{
			int newline_index = buffer_.indexOf('\n', buffer_read_pos_);
			if (newline_index != -1)
			{
				int line_length = newline_index - buffer_read_pos_ + 1;
				if (maxlen > 0) line_length = qMin(line_length, (int)maxlen);

				output = buffer_.mid(buffer_read_pos_, line_length);
				buffer_read_pos_ += output.size();
				cursor_position_ = remote_position_ - (buffer_.size() - buffer_read_pos_);
				break;
			}

			if (remote_position_ >= file_size_)
			{
				if (buffer_read_pos_ >= buffer_.size()) return QByteArray();

				output = buffer_.mid(buffer_read_pos_);
				buffer_read_pos_ = buffer_.size();
				cursor_position_ = remote_position_;
				break;
			}

			if (buffer_read_pos_ > buffer_.size() / 2)
			{
				buffer_ = buffer_.mid(buffer_read_pos_);
				buffer_read_pos_ = 0;
			}

			qint64 end = qMin(remote_position_ + chunkSize() - 1, file_size_ - 1);
			QByteArray chunk = httpRangeRequest(remote_position_, end);
			if (chunk.isEmpty()) return QByteArray();

			remote_position_ += chunk.size();
			buffer_.append(chunk);
		}
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
	else if (mode_==URL_GZ)
	{        
		return remote_gz_finished_ && (decompressed_buffer_pos_ >= decompressed_buffer_.size());
	}

	//mode_==URL
    if (file_size_ == -1) THROW(ProgrammingException, QString(__FUNCTION__) + " called, on non existing file '" + file_name_ + "!");
    bool no_more_remote_data = (file_size_ != -1 && remote_position_ >= file_size_);
	bool buffer_consumed = (buffer_read_pos_ >= buffer_.size());
	return (no_more_remote_data && buffer_consumed) || (cursor_position_ >= file_size_);
}

bool VersatileFile::exists()
{
	if (mode_==LOCAL || mode_==LOCAL_GZ)
	{
		return QFile(file_name_).exists();
	}

    return file_exists_;
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

    buffer_.clear();
    decompressed_buffer_.clear();
    decompressed_buffer_pos_ = 0;
    cursor_position_ = 0;
    remote_position_ = 0;
    remote_gz_finished_ = false;
}

qint64 VersatileFile::pos() const
{
	if (!is_open_) THROW(ProgrammingException, QString(__FUNCTION__) + " called, on not open file '" + file_name_ + "!");

	if (mode_==LOCAL)
	{
		return local_source_.data()->pos();
	}
    else if ((mode_==LOCAL_GZ) || (mode_==URL_GZ))
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
    else if ((mode_==LOCAL_GZ) || (mode_==URL_GZ))
	{
        if (pos==0)
        {
            close();
            return open();
        }

        THROW(NotImplementedException, "VersatileFile::seek is not fully implemented for GZ files, only resetting to the beginning of the file is supported!");
    }

    if (pos < 0 || (file_size_ != -1 && pos > file_size_)) return false;

    cursor_position_ = pos;
    remote_position_ = pos;
    buffer_.clear();
    buffer_read_pos_ = 0;
    return true;
}

qint64 VersatileFile::size()
{
	if (mode_==LOCAL)
	{
		return local_source_.data()->size();
	}
	else if (mode_==LOCAL_GZ)
	{
        return QFileInfo(file_name_).size();
	}

    return file_size_;
}

QString VersatileFile::fileName() const
{
	return file_name_;
}

QByteArray VersatileFile::httpRangeRequest(qint64 start, qint64 end)
{
    QNetworkRequest request((QUrl(file_name_)));
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    request.setRawHeader("Range", "bytes=" + QByteArray::number(start) + "-" + QByteArray::number(end));

    QNetworkReply* reply = net_mgr_.get(request);
    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    QByteArray data;
    if (reply->error() == QNetworkReply::NoError)
    {
        data = reply->readAll();
    }
    reply->deleteLater();
    return data;
}

void VersatileFile::checkRemoteFile()
{
    QNetworkRequest request((QUrl(file_name_)));
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);

    QNetworkReply* reply = net_mgr_.head(request);
    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    file_exists_ = (reply->error() == QNetworkReply::NoError);
    if (file_exists_ && file_size_ == -1)
    {
        file_size_ = reply->header(QNetworkRequest::ContentLengthHeader).toLongLong();
    }
    reply->deleteLater();
}

QByteArray VersatileFile::decompressGzip(const QByteArray &compressed)
{
    if (compressed.isEmpty()) return QByteArray();

    std::vector<uint8_t> buffer;
    buffer.resize(1024 * 1024); // initial output buffer
    size_t out_offset = 0;

    const uint8_t* next_in = reinterpret_cast<const uint8_t*>(compressed.constData());
    size_t avail_in = compressed.size();

    z_stream strm{};
    if (inflateInit2(&strm, 16 + MAX_WBITS) != Z_OK)
    {
        THROW(ProgrammingException, QString(__FUNCTION__) + " failed to init zlib for '" + file_name_ + "!");
    }

    int ret = Z_OK;

    while (avail_in > 0)
    {
        strm.next_in  = const_cast<Bytef*>(next_in);
        strm.avail_in = static_cast<uInt>(avail_in);

        do
        {
            // grow buffer if needed
            if (buffer.size() - out_offset < 4096) buffer.resize(buffer.size() * 2);

            strm.next_out  = buffer.data() + out_offset;
            strm.avail_out = buffer.size() - out_offset;

            ret = inflate(&strm, Z_NO_FLUSH);

            if (ret != Z_OK && ret != Z_STREAM_END && ret != Z_BUF_ERROR)
            {
                inflateEnd(&strm);
                THROW(ProgrammingException, QString(__FUNCTION__) + " for the file '" + file_name_ + " inflate failed with code: " + QString::number(ret));
            }

            size_t produced = (buffer.size() - out_offset) - strm.avail_out;
            out_offset += produced;

        }
        while (strm.avail_out == 0);

        size_t consumed = avail_in - strm.avail_in;
        next_in += consumed;
        avail_in -= consumed;

        if (ret == Z_STREAM_END)
        {
            //reset stream to process the next gzip member
            if (avail_in > 0)
            {
                inflateReset(&strm);
            }
            else
            {
                break; //fully done
            }
        }
    }

    inflateEnd(&strm);

    QByteArray result;
    result.resize(out_offset);
    memcpy(result.data(), buffer.data(), out_offset);
    return result;
}


