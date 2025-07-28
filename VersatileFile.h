#ifndef VERSATILEFILE_H
#define VERSATILEFILE_H

#include "cppCORE_global.h"
#include <QNetworkProxy>
#include <QFile>
#include <zlib.h>
#include "Exceptions.h"
#include "HttpRequestHandler.h"

//File class that can handle plain text files, gzipped text files and URLs
class CPPCORESHARED_EXPORT VersatileFile
    : public QObject
{
    Q_OBJECT

public:
	///Constructor for local file, local gzipped file or URL
	VersatileFile(QString file_name);
	///Constructor for stdin/stderr
	VersatileFile(QString file_name, FILE* f);
	///Destructor. Calls close() to frees all resources.
	~VersatileFile();

	///Open file. Returns false if file could not be opened. Open mode is only used in LOCAL mode.
	///Note: Use QIODevice::Text in addition to the read/write mode for text files!
	bool open(QIODevice::OpenMode mode = QFile::ReadOnly, bool throw_on_error = true);
	///Returns the open mode (for local files).
	QIODevice::OpenMode openMode() const;
	///Returns the mode.
	enum Mode { LOCAL, LOCAL_GZ, URL};
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
	qint64 file_size_;
	qint64 cursor_position_;
	QSharedPointer<QFile> readline_pointer_;

	void checkResponse(QByteArray& response) const;
    void addCommonHeaders(HttpHeaders &request_headers);

    ServerReply sendHeadRequest();
    ServerReply sendGetRequestText();
    ServerReply sendByteRangeRequestText(qint64 start, qint64 end);

	qint64 getFileSize();
};


#endif // VERSATILEFILE_H
