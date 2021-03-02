#include "VersatileFile.h"
#include "HttpRequestHandler.h"

VersatileFile::VersatileFile(const QString& name, bool stdin_if_empty)	
{
	if (!name.toLower().startsWith("http"))
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

		device_ = file_;
	}
	else
	{
		QString reply = HttpRequestHandler(HttpRequestHandler::NONE).get(name);
		reply_data_ = reply.toLocal8Bit();
		buffer_ = QSharedPointer<QBuffer>(new QBuffer(&reply_data_));
		buffer_->open(QBuffer::ReadOnly | QBuffer::Text);
		if (!buffer_->isOpen())
		{
			THROW(FileAccessException, "Could not open remote file for reading: '" + name + "'!");
		}

		device_ = buffer_;
	}
}

VersatileFile::~VersatileFile()
{	
	device_->close();
}

void VersatileFile::close()
{
	checkIfOpen();
	device_->close();
}

bool VersatileFile::isSequential() const
{
	checkIfOpen();
	return device_->isSequential();
}

qint64 VersatileFile::pos() const
{
	checkIfOpen();
	return device_->pos();
}

bool VersatileFile::seek(qint64 offset)
{
	checkIfOpen();
	return device_->seek(offset);
}

qint64 VersatileFile::size() const
{
	checkIfOpen();
	return device_->size();
}

bool VersatileFile::atEnd() const
{
	return device_->atEnd();
}

bool VersatileFile::exists()
{
	return device_->isOpen();
}

QByteArray VersatileFile::readLine(qint64 maxlen)
{
	checkIfOpen();
	return device_->readLine(maxlen);
}

QByteArray VersatileFile::readAll()
{
	checkIfOpen();
	return device_->readAll();
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

void VersatileFile::checkIfOpen() const
{
	if (!device_->isOpen())
	{
		THROW(FileAccessException, "IODevice in VersatileFile is closed!");
	}
}
