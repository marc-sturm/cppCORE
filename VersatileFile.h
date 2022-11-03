#ifndef VERSATILEFILE_H
#define VERSATILEFILE_H

#include "cppCORE_global.h"
#include <QSslSocket>
#include <QFile>
#include "Exceptions.h"

class CPPCORESHARED_EXPORT VersatileFile
{
public:
	VersatileFile(const QString &file_name);
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
	QSslSocket *socket_;
	QByteArray reply_data_;
	QSharedPointer<QFile> local_source_;

	QString file_name_;
	void checkIfOpen() const;
	void checkResponse(QByteArray& response) const;

	bool is_local_;
	QString server_path_;
	QString host_name_;
	quint16 server_port_;
	qint64 file_size_;
	qint64 cursor_position_;
	bool headers_processed_;

	QList<QByteArray> buffer_;

	bool isLocal() const;
	bool isEncrypted() const;
	quint16 getPortNumber();
	QByteArray createHeadRequestText();
	QByteArray createGetRequestText();
	QByteArray createByteRangeRequestText(qint64 start, qint64 end);
	void initiateRequest(const QByteArray& http_request);
	QByteArray readAllViaSocket(const QByteArray &http_request);
	QByteArray readLineViaSocket(const QByteArray& http_request, qint64 maxlen = 0);
	qint64 getFileSize();
	QByteArray readResponseWithoutHeaders(const QByteArray &http_request);
	QByteArray readLineWithoutHeaders(qint64 maxlen = 0);
};


#endif // VERSATILEFILE_H
