#ifndef VERSATILETEXTSTREAM_H
#define VERSATILETEXTSTREAM_H

#include "cppCORE_global.h"
#include "VersatileFile.h"
#include <QTextStream>

class CPPCORESHARED_EXPORT VersatileTextStream
{
public:
	VersatileTextStream(const QString &file_name);
	~VersatileTextStream();

	bool atEnd();
	QString readLine(qint64 maxlen = 0);

private:
	bool isLocal();
	QTextStream stream_;
	QSharedPointer<VersatileFile> remote_file_;
	QSharedPointer<QFile> local_file_;
	QString file_name_;
};

#endif // VERSATILETEXTSTREAM_H
