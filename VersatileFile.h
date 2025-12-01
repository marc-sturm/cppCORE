#ifndef VERSATILEFILE_H
#define VERSATILEFILE_H

#include "cppCORE_global.h"
#include <QNetworkProxy>
#include <QFile>
#include <QNetworkAccessManager>
#include <QSharedPointer>
#include <QByteArray>
#include <QObject>
#include <zlib.h> //TODO Marc try libdeflate instead of zlib

//File class that can handle plain text files, gzipped text files and URLs.
//If you need QString output with proper handling of the encoding, use VersatileTextStream.
class CPPCORESHARED_EXPORT VersatileFile
    : public QObject
{
    Q_OBJECT

public:
	///Constructor for local file, local gzipped file or URL
	VersatileFile(QString file_name, bool stdin_if_empty = false);
	///Destructor. Calls close() to frees all resources.
	~VersatileFile();

	///Open file. Returns false if file could not be opened. Open mode is only used in LOCAL mode.
	///Note: Use QIODevice::Text in addition to the read/write mode for text files!
	bool open(QIODevice::OpenMode mode = QFile::ReadOnly, bool throw_on_error = true);    

	///Returns the open mode (for local files).
	QIODevice::OpenMode openMode() const;
	///Returns the mode.
	enum Mode { LOCAL, LOCAL_GZ, URL, URL_GZ};
	Mode mode() const
	{
		return mode_;
	}

	//set output buffer size, i.e. the maximum line size. Call before opening the file!
	void setGzBufferSize(int bytes);
	//set internal buffer size, i.e. the buffer that is internally used by zlib. Increasing this buffer should improve reading speed. Call before opening the file!
	void setGzBufferSizeInternal(int bytes);

	bool isOpen() const { return is_open_; }
	bool isReadable() const;

	QByteArray read(qint64 maxlen = 0);
	QByteArray readAll();
    QByteArray readLine(bool trim_line_endings = false);

	bool atEnd() const;
	bool exists();
	void close();

	qint64 pos() const;
	bool seek(qint64 pos);
	qint64 size();

	QString fileName() const;

private:
    QNetworkAccessManager net_mgr_;
    QNetworkProxy proxy_;
	QString file_name_;
	FILE* file_stream_pointer_;
	Mode mode_ = LOCAL;
	bool is_open_;

	//members for LOCAL mode
	QSharedPointer<QFile> local_source_;

	//members for LOCAL_GZ mode
	int gz_buffer_size_ = 1048576; //1MB buffer
	int gz_buffer_size_internal_ = 16*1048576; //16MB buffer
	char* gz_buffer_ = nullptr;
	gzFile gz_stream_ = nullptr;

    //members for URL mode
    QByteArray buffer_;
    qint64 cursor_position_ = 0; // position in a VersatileFile, as if we are reading QFile
    qint64 remote_position_ = 0; // where we are in the remote file while we read and save its content into a buffer)
    qint64 buffer_read_pos_ = 0; // position within the read buffer
	QSharedPointer<QFile> readline_pointer_;

    //members for all modes
    qint64 file_size_ = -1;
    bool file_exists_ = false;

    //members for remote decompression
    static constexpr qint64 chunkSize() { return 200 * 1024 * 1024; } // 200Mb

    bool remote_gz_finished_ = false;
    QByteArray decompressed_buffer_;
    qint64 decompressed_buffer_pos_ = 0;

    //gets a chunk from the remote file
    QByteArray httpRangeRequest(qint64 start, qint64 end);
    //checks if remote file exists and gets the file size from the header
    void checkRemoteFile();
    QByteArray decompressGzip(const QByteArray &compressed);
};


#endif // VERSATILEFILE_H
