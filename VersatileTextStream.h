#ifndef VERSATILETEXTSTREAM_H
#define VERSATILETEXTSTREAM_H

#include "cppCORE_global.h"
#include "VersatileFile.h"

//Text stream wrapper around VersatileFile, i.e. it returns QString instead of QByteArray.
//Note: It is assumed that special caracters is encoded using UTF8 encoding.
class CPPCORESHARED_EXPORT VersatileTextStream
{
public:
	VersatileTextStream(QString file_name);

	bool atEnd() const
	{
		return file_.atEnd();
	}

	QString readLine(bool trim_line_endings = true)
	{
		return QString::fromUtf8(file_.readLine(trim_line_endings));
	}

	VersatileFile::Mode mode()
	{
		return file_.mode();
	}

private:
	QString file_name_;
	VersatileFile file_;
};

#endif // VERSATILETEXTSTREAM_H
