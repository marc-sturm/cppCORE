#include "VersatileTextStream.h"

VersatileTextStream::VersatileTextStream(const QString &file_name)
	: file_name_(file_name)
{
	if (isLocal())
	{
		QSharedPointer<QFile> file(new QFile(file_name_));
		local_file_ = file;
		if (local_file_.data()->open(QFile::ReadOnly | QIODevice::Text))
		{
			stream_.setDevice(local_file_.data());
		}
		else
		{
			THROW(FileAccessException, "Could not open local file "+file_name_+" for streaming!");
		}
	}
	else
	{
		QSharedPointer<VersatileFile> file(new VersatileFile(file_name_));
		remote_file_ = file;
		if (!remote_file_.data()->open(QFile::ReadOnly | QIODevice::Text))
		{
			THROW(FileAccessException, "Could not open remote file "+file_name_+" for streaming!");
		}
	}
}

VersatileTextStream::~VersatileTextStream()
{
}

bool VersatileTextStream::atEnd()
{
	if (isLocal())
	{
		return stream_.atEnd();
	}

	if (remote_file_.isNull()) THROW(FileAccessException, "Remote file "+file_name_+" is not set!");
	return remote_file_.data()->atEnd();
}

QString VersatileTextStream::readLine(qint64 maxlen)
{
	if (isLocal())
	{
		return stream_.readLine(maxlen);
	}

	if (remote_file_.isNull()) THROW(FileAccessException, "Remote file "+file_name_+" is not set!");
	return remote_file_.data()->readLine(maxlen);
}

bool VersatileTextStream::isLocal()
{
	return (!file_name_.startsWith("http", Qt::CaseInsensitive));
}
