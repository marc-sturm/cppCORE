#ifndef VERSATILEFILE_H
#define VERSATILEFILE_H

#include "cppCORE_global.h"
#include <QIODevice>
#include <QFileDevice>
#include <QFile>
#include <QBuffer>
#include "Exceptions.h"

class CPPCORESHARED_EXPORT VersatileFile
{
public:
	VersatileFile(const QString &name, bool stdin_if_empty=false);
	~VersatileFile();

	QByteArray readLine(qint64 maxlen = 0);
	QByteArray readAll();
	bool atEnd();
	bool exists();
	bool isLocalFile(QString src);

private:
	QString source_;
	QByteArray reply_data_;
	QSharedPointer<QBuffer> buffer_;
	QSharedPointer<QFile> file_;
};

#endif // VERSATILEFILE_H
