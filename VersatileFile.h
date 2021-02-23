#ifndef VERSATILEFILE_H
#define VERSATILEFILE_H

#include "cppCORE_global.h"
#include <QIODevice>
#include <QFileDevice>
#include <QFile>
#include <QBuffer>
#include "Exceptions.h"

class CPPCORESHARED_EXPORT VersatileFile : public QIODevice
{
	Q_OBJECT

public:
	VersatileFile(const QString &name, bool stdin_if_empty=false);
	~VersatileFile();

	QByteArray readLine(qint64 maxlen = 0);
	QByteArray readAll();
	bool atEnd() const override;
	bool exists();
	bool isLocalFile(QString src) const;
	void close() override;


	bool isSequential() const override;
	qint64 pos() const override;
	bool seek(qint64 offset) override;
	qint64 size() const override;

protected:
	qint64 readData(char *data, qint64 maxlen) override;
	qint64 writeData(const char *data, qint64 len) override;
	qint64 readLineData(char *data, qint64 maxlen) override;

private:
	bool is_local_file_;
	QByteArray reply_data_;
	QSharedPointer<QBuffer> buffer_;
	QSharedPointer<QFile> file_;
};

#endif // VERSATILEFILE_H
