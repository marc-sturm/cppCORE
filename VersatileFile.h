#ifndef VERSATILEFILE_H
#define VERSATILEFILE_H

#include "cppCORE_global.h"
#include <QIODevice>
#include <QFile>
#include <QBuffer>
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

	qint64 read(char *data, qint64 maxlen);
	QByteArray read(qint64 maxlen);
	QByteArray readAll();
	qint64 readLine(char *data, qint64 maxlen);
	QByteArray readLine(qint64 maxlen = 0);
	bool canReadLine() const;

	bool atEnd() const;
	bool exists();

	void close();
	bool reset();

	bool isSequential() const;
	qint64 pos() const;
	bool seek(qint64 offset);
	qint64 size() const;

	QTextStream& createTextStream();

private:
	QByteArray reply_data_;
	QSharedPointer<QFile> file_;
	QSharedPointer<QBuffer> buffer_;
	QSharedPointer<QIODevice> device_;
	QString file_name_;	
	void checkIfOpen() const;

	QTextStream text_stream;
};

#endif // VERSATILEFILE_H
