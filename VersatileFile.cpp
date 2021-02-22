#include "VersatileFile.h"
#include "HttpRequestHandler.h"

VersatileFile::VersatileFile(const QString& name, bool stdin_if_empty)
		: source_(name)
{
	if (isLocalFile(name))
	{
		file_ = QSharedPointer<QFile>(new QFile(name));

		if (stdin_if_empty && name=="")
		{
			file_->open(stdin, QFile::ReadOnly | QIODevice::Text);
		}
		else if (!file_->open(QFile::ReadOnly | QIODevice::Text))
		{
			THROW(FileAccessException, "Could not open file for reading: '" + name + "'!");
		}
	}
	else
	{
		QString reply = HttpRequestHandler(HttpRequestHandler::NONE).get(name);
		reply_data_ = reply.toLocal8Bit();

		buffer_ = QSharedPointer<QBuffer>(new QBuffer(&reply_data_));
		buffer_->open(QBuffer::ReadOnly | QBuffer::Text);
	}
}

VersatileFile::~VersatileFile()
{
}

bool VersatileFile::isLocalFile(QString src)
{
	if (src.toLower().startsWith("http"))
	{
		return false;
	}
	return true;
}

void VersatileFile::close()
{
	if (isLocalFile(source_))
	{
		file_->close();
	}
	else
	{
		if (!buffer_->isOpen())
		{
			buffer_->close();
		}
	}
}

bool VersatileFile::atEnd()
{
	if (isLocalFile(source_))
	{
		return file_->atEnd();
	}
	else
	{
		if (!buffer_->isOpen())
		{
			buffer_->open(QBuffer::ReadOnly);
		}
		return buffer_->atEnd();
	}
}

bool VersatileFile::exists()
{
	if (isLocalFile(source_))
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
	if (isLocalFile(source_))
	{
		return file_->readLine();
	}
	else
	{
		if (!buffer_->isOpen())
		{
			buffer_->open(QBuffer::ReadOnly);
		}
		return buffer_->readLine();

	}
	return QByteArray();
}

QByteArray VersatileFile::readAll()
{
	if (isLocalFile(source_))
	{
		return file_->readAll();
	}
	else
	{
		return buffer_->readAll();
	}
}
