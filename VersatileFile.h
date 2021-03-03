#ifndef VERSATILEFILE_H
#define VERSATILEFILE_H

#include "cppCORE_global.h"
#include <QIODevice>
#include <QFileDevice>
#include <QFile>
#include <QBuffer>
#include <QTextStream>
#include "Exceptions.h"

class CPPCORESHARED_EXPORT VersatileFile : public QIODevice
{
	Q_OBJECT

public:
	VersatileFile(const QString &name, bool stdin_if_empty=false);
	~VersatileFile();

	OpenMode openMode() const;

	bool open(OpenMode mode) override;

	void setTextModeEnabled(bool enabled);
	bool isTextModeEnabled() const;

	bool isOpen() const;
	bool isReadable() const;
	bool isWritable() const;

	qint64 read(char *data, qint64 maxlen);
	QByteArray read(qint64 maxlen);
	QByteArray readAll();
	qint64 readLine(char *data, qint64 maxlen);
	QByteArray readLine(qint64 maxlen = 0);
	bool canReadLine() const override;

	bool atEnd() const override;
	bool exists();
	bool isLocalFile(QString src) const;
	void close() override;
	bool reset() override;

	qint64 bytesAvailable() const override;
	qint64 bytesToWrite() const override;


	bool isSequential() const override;
	qint64 pos() const override;
	bool seek(qint64 offset) override;
	qint64 size() const override;

	void ungetChar(char c);
	bool putChar(char c);
	bool getChar(char *c);

	QString errorString() const;

	bool waitForReadyRead(int msecs) override;
	bool waitForBytesWritten(int msecs) override;

	QByteArray toByteArray();

protected:
	qint64 readData(char *data, qint64 maxlen) override;
	qint64 writeData(const char *data, qint64 len) override;
	qint64 readLineData(char *data, qint64 maxlen) override;

private:
	QByteArray reply_data_;
	QSharedPointer<QFile> file_;
	QSharedPointer<QBuffer> buffer_;
	QSharedPointer<QIODevice> device_;
	void checkIfOpen() const;
};

#endif // VERSATILEFILE_H
