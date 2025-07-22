#ifndef VERSATILEFILE_H
#define VERSATILEFILE_H

#include "cppCORE_global.h"
#include <QNetworkProxy>
#include <QFile>
#include <zlib.h>
#include "Exceptions.h"
#include "HttpRequestHandler.h"

class CPPCORESHARED_EXPORT VersatileFile
    : public QObject
{
    Q_OBJECT

public:
	VersatileFile(QString file_name);

	~VersatileFile();

	bool open(QIODevice::OpenMode mode);
	bool open(FILE *f, QIODevice::OpenMode ioFlags);

	QIODevice::OpenMode openMode() const;

	bool isOpen() const;
	bool isReadable() const;

	QByteArray read(qint64 maxlen = 0);
	QByteArray readAll();
	QByteArray readLine(qint64 maxlen = 0);

	bool atEnd() const;
	bool exists();
	void close();

	qint64 pos() const;
	bool seek(qint64 pos);
	qint64 size();

	QString fileName() const;

private:
    QNetworkProxy proxy_;
	QSharedPointer<QFile> local_source_;

	QString file_name_;
	void checkIfOpen() const;
	void checkResponse(QByteArray& response) const;

	bool is_local_;
	qint64 file_size_;
	qint64 cursor_position_;

	QSharedPointer<QFile> readline_pointer_;
	bool isLocal() const;	
    void addCommonHeaders(HttpHeaders &request_headers);

    ServerReply sendHeadRequest();
    ServerReply sendGetRequestText();
    ServerReply sendByteRangeRequestText(qint64 start, qint64 end);

	qint64 getFileSize();
};


#endif // VERSATILEFILE_H
