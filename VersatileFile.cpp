#include "VersatileFile.h"
#include "CustomProxyService.h"
#include "Helper.h"

VersatileFile::VersatileFile(const QString& url)
    : file_name_(url)
    , proxy_(QNetworkProxy::NoProxy)
{

    if (!isLocal())
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

bool VersatileFile::open(QIODevice::OpenMode mode)
{
    if (isLocal())
    {
        local_file_ = QSharedPointer<QFile>(new QFile(file_name_));
        return local_file_.data()->open(mode);
    }

    cursor_position_ = 0;
    buffer_.clear();
    return exists();
}

bool VersatileFile::open(FILE* f, QIODevice::OpenMode flags)
{
    if (isLocal())
    {
        local_file_ = QSharedPointer<QFile>(new QFile(file_name_));
        return local_file_.data()->open(f, flags);
    }

    cursor_position_ = 0;
    buffer_.clear();
    return exists();
}

bool VersatileFile::isReadable() const
{
    if (isLocal()) return QFile::exists(file_name_);
    return file_exists_;
}

void VersatileFile::close()
{
    if (isLocal() && local_file_)
    {
        local_file_->close();
        return;
    }

    if (zstream_initialized_)
    {
        inflateEnd(&zstream_);
        zstream_initialized_ = false;
    }

    buffer_.clear();
    decompressed_buffer_.clear();
    decompressed_buffer_pos_ = 0;
    cursor_position_ = 0;
    remote_position_ = 0;
    remote_gz_finished_ = false;
}

QByteArray VersatileFile::readAll()
{
    if (isLocal())
    {
        return local_file_.data()->readAll();
    }

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
    return data;
}

QByteArray VersatileFile::read(qint64 maxlen)
{
    if (isLocal()) return local_file_.data()->read(maxlen);

    if (maxlen <= 0) maxlen = CHUNK_SIZE;

    QByteArray result;
    while (result.size() < maxlen)
    {
        qint64 to_read = qMin(CHUNK_SIZE, maxlen - result.size());
        QByteArray chunk = httpRangeRequest(cursor_position_, cursor_position_ + to_read - 1);
        if (chunk.isEmpty()) break;

        result.append(chunk);
        cursor_position_ += chunk.size();
    }

    return result;
}

QByteArray VersatileFile::readLine(qint64 maxlen)
{
    if (isLocal())
    {
        QByteArray line = local_file_.data()->readLine(maxlen);
        cursor_position_ += line.size();
        return line;
    }

    // GZ compressed remote file
    if (file_name_.endsWith(".gz"))
    {
        if (!zstream_initialized_)
        {
            memset(&zstream_, 0, sizeof(zstream_));
            if (inflateInit2(&zstream_, 16 + MAX_WBITS) != Z_OK) return QByteArray();
            zstream_initialized_ = true;
        }

        while (true)
        {
            int newline_index = decompressed_buffer_.indexOf('\n', decompressed_buffer_pos_);
            if (newline_index != -1)
            {
                int line_length = newline_index - decompressed_buffer_pos_ + 1;
                if (maxlen > 0) line_length = qMin(line_length, (int)maxlen);

                QByteArray line = decompressed_buffer_.mid(decompressed_buffer_pos_, line_length);
                decompressed_buffer_pos_ += line_length;
                cursor_position_ += line.size();
                return line;
            }

            if (remote_gz_finished_)
            {
                if (decompressed_buffer_pos_ >= decompressed_buffer_.size()) return QByteArray();

                QByteArray line = decompressed_buffer_.mid(decompressed_buffer_pos_);
                cursor_position_ += line.size();
                decompressed_buffer_pos_ = decompressed_buffer_.size();
                return line;
            }

            if (decompressed_buffer_pos_ > decompressed_buffer_.size() / 2)
            {
                decompressed_buffer_ = decompressed_buffer_.mid(decompressed_buffer_pos_);
                decompressed_buffer_pos_ = 0;
            }

            qint64 end = qMin(remote_position_ + CHUNK_SIZE - 1, size() - 1);
            QByteArray compressed_chunk = httpRangeRequest(remote_position_, end);
            if (compressed_chunk.isEmpty())
            {
                remote_gz_finished_ = true;
                continue;
            }

            remote_position_ += compressed_chunk.size();

            QByteArray out;
            out.resize(1024 * 1024);
            zstream_.next_in = reinterpret_cast<Bytef*>(compressed_chunk.data());
            zstream_.avail_in = compressed_chunk.size();

            while (zstream_.avail_in > 0)
            {
                if (out.size() - zstream_.total_out < 1024) out.resize(out.size() * 2);

                zstream_.next_out = reinterpret_cast<Bytef*>(out.data()) + zstream_.total_out;
                zstream_.avail_out = out.size() - zstream_.total_out;

                int ret = inflate(&zstream_, Z_NO_FLUSH);
                if (ret == Z_STREAM_END)
                {
                    remote_gz_finished_ = true;
                    break;
                }
                if (ret != Z_OK) break;
            }

            out.resize(zstream_.total_out);
            decompressed_buffer_.append(out);
        }
    }

    // Regular remote text file
    while (true)
    {
        int newline_index = buffer_.indexOf('\n', buffer_read_pos_);
        if (newline_index != -1)
        {
            int line_length = newline_index - buffer_read_pos_ + 1;
            if (maxlen > 0) line_length = qMin(line_length, (int)maxlen);

            QByteArray line = buffer_.mid(buffer_read_pos_, line_length);
            buffer_read_pos_ += line.size();
            cursor_position_ = remote_position_ - (buffer_.size() - buffer_read_pos_);
            return line;
        }

        if (remote_position_ >= file_size_)
        {
            if (buffer_read_pos_ >= buffer_.size()) return QByteArray();

            QByteArray leftover = buffer_.mid(buffer_read_pos_);
            buffer_read_pos_ = buffer_.size();
            cursor_position_ = remote_position_;
            return leftover;
        }

        if (buffer_read_pos_ > buffer_.size() / 2)
        {
            buffer_ = buffer_.mid(buffer_read_pos_);
            buffer_read_pos_ = 0;
        }

        qint64 end = qMin(remote_position_ + CHUNK_SIZE - 1, file_size_ - 1);
        QByteArray chunk = httpRangeRequest(remote_position_, end);
        if (chunk.isEmpty()) return QByteArray();

        remote_position_ += chunk.size();
        buffer_.append(chunk);
    }
}

bool VersatileFile::atEnd() const
{
    if (isLocal()) return local_file_.data()->atEnd();

    bool no_more_remote_data = (file_size_ != -1 && remote_position_ >= file_size_);
    bool buffer_consumed = (buffer_read_pos_ >= buffer_.size());

    return (no_more_remote_data && buffer_consumed) || (cursor_position_ >= (file_size_));
}

bool VersatileFile::exists()
{
    if (isLocal()) return QFile::exists(file_name_);

    checkRemoteFile();
    return file_exists_;
}

qint64 VersatileFile::pos() const
{
    return cursor_position_;
}

bool VersatileFile::seek(qint64 pos)
{
    if (isLocal()) return local_file_.data()->seek(pos);

    if (pos < 0 || (file_size_ != -1 && pos > file_size_)) return false;

    cursor_position_ = pos;
    remote_position_ = pos;
    buffer_.clear();
    buffer_read_pos_ = 0;
    return true;
}

qint64 VersatileFile::size()
{
    if (isLocal()) return local_file_.data()->size();
    if (file_size_ == -1) exists();
    return file_size_;
}

QString VersatileFile::fileName() const
{
    return file_name_;
}

bool VersatileFile::isLocal() const
{
    return !Helper::isHttpUrl(file_name_);
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
