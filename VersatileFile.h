#ifndef VERSATILEFILE_H
#define VERSATILEFILE_H

#include "cppCORE_global.h"
#include <QNetworkProxy>
#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QSharedPointer>
#include <QByteArray>
#include <QObject>
#include <zlib.h>
#include "Exceptions.h"

class CPPCORESHARED_EXPORT VersatileFile : public QObject
{
    Q_OBJECT

public:
    explicit VersatileFile(const QString& url);
    ~VersatileFile();

    bool open(QIODevice::OpenMode mode);
    bool open(FILE *f, QIODevice::OpenMode flags);

    bool isReadable() const;

    void close();

    QByteArray readAll();
    QByteArray read(qint64 maxlen = 0);
    QByteArray readLine(qint64 maxlen = 0);
    bool atEnd() const;
    bool exists();

    qint64 pos() const;
    bool seek(qint64 pos);
    qint64 size();
    QString fileName() const;

private:
    bool isLocal() const;
    QByteArray httpRangeRequest(qint64 start, qint64 end);
    void checkRemoteFile();

private:
    QString file_name_;
    QSharedPointer<QFile> local_file_;
    QNetworkAccessManager net_mgr_;
    QNetworkProxy proxy_;

    QByteArray buffer_;

    qint64 file_size_ = -1;
    qint64 cursor_position_ = 0; // position in a VersatileFile, as if we are reading QFile
    qint64 remote_position_ = 0; // where we are in the remote file while we read and save its content into a buffer)
    qint64 buffer_read_pos_ = 0; // position within the read buffer
    bool file_exists_ = false;

    static constexpr qint64 CHUNK_SIZE = 200 * 1024 * 1024; // 200Mb

    z_stream zstream_ = {};
    bool zstream_initialized_ = false;
    bool remote_gz_finished_ = false;
    QByteArray decompressed_buffer_;
    qint64 decompressed_buffer_pos_ = 0;
};

#endif // VERSATILEFILE_H_
