#include "VersatileFile.h"
#include "HttpRequestHandler.h"

VersatileFile::VersatileFile(const QString& name, bool stdin_if_empty)
	:is_local_file_(true)
{
	if (name.toLower().startsWith("http"))
	{
		is_local_file_ = false;
	}

	if (is_local_file_)
	{
		file_ = QSharedPointer<QFile>(new QFile(name));

		if (stdin_if_empty && name=="")
		{
			file_->open(stdin, QFile::ReadOnly | QIODevice::Text);
		}
		else if (!file_->open(QFile::ReadOnly | QIODevice::Text))
		{
			THROW(FileAccessException, "Could not open local file for reading: '" + name + "'!");
		}
	}
	else
	{
		QString reply = HttpRequestHandler(HttpRequestHandler::NONE).get(name);
		reply_data_ = reply.toLocal8Bit();

		buffer_ = QSharedPointer<QBuffer>(new QBuffer(&reply_data_));
		if (!buffer_->open(QBuffer::ReadOnly | QBuffer::Text))
		{
			THROW(FileAccessException, "Could not open remote file for reading: '" + name + "'!");
		}
	}
}

VersatileFile::~VersatileFile()
{
}

void VersatileFile::close()
{
	if (is_local_file_)
	{
		file_->close();
	}
	else
	{
		buffer_->close();
	}
}

bool VersatileFile::isSequential() const
{
	if (is_local_file_)
	{
		return file_->isSequential();
	}
	else
	{
		return buffer_->isSequential();
	}
}

qint64 VersatileFile::pos() const
{
	if (is_local_file_)
	{
		return file_->pos();
	}
	else
	{
		return buffer_->pos();
	}
}

bool VersatileFile::seek(qint64 offset)
{
	if (is_local_file_)
	{
		return file_->seek(offset);
	}
	else
	{
		return buffer_->seek(offset);
	}
}

qint64 VersatileFile::size() const
{
	if (is_local_file_)
	{
		return file_->size();
	}
	else
	{
		return buffer_->size();
	}
}

bool VersatileFile::atEnd() const
{
	if (is_local_file_)
	{
		return file_->atEnd();
	}
	else
	{
		return buffer_->atEnd();
	}
}

bool VersatileFile::exists()
{
	if (is_local_file_)
	{
		return file_->exists();
	}
	else
	{
		return !buffer_.isNull();
	}
}

QByteArray VersatileFile::readLine(qint64 maxlen)
{
	if (is_local_file_)
	{
		return file_->readLine(maxlen);
	}
	else
	{		
		return buffer_->readLine(maxlen);

	}
	return QByteArray();
}

QByteArray VersatileFile::readAll()
{
	if (is_local_file_)
	{
		return file_->readAll();
	}
	else
	{
		return buffer_->readAll();
	}
}

qint64 VersatileFile::readData(char* data, qint64 maxlen)
{
	return 0;
}

qint64 VersatileFile::writeData(const char* data, qint64 len)
{
	return 0;
}

qint64 VersatileFile::readLineData(char* data, qint64 maxlen)
{
	return 0;
}
