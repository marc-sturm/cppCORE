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
		else if (!file_->open(QIODevice::ReadOnly | QIODevice::Text))
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
	if (device_->isOpen()) device_->close();
}

QIODevice::OpenMode VersatileFile::openMode() const
{
	return device_->openMode();
}

bool VersatileFile::open(QIODevice::OpenMode mode)
{
	return device_->open(mode);
}

void VersatileFile::setTextModeEnabled(bool enabled)
{
	device_->setTextModeEnabled(enabled);
}

bool VersatileFile::isTextModeEnabled() const
{
	return device_->isTextModeEnabled();
}

bool VersatileFile::isOpen() const
{
	return device_->isOpen();
}

bool VersatileFile::isReadable() const
{
	return device_->isReadable();
}

bool VersatileFile::isWritable() const
{
	return device_->isWritable();
}

void VersatileFile::close()
{
	checkIfOpen();
	device_->close();
}

bool VersatileFile::reset()
{
	checkIfOpen();
	return device_->reset();
}

qint64 VersatileFile::bytesAvailable() const
{
	return device_->bytesAvailable();
}

qint64 VersatileFile::bytesToWrite() const
{
	return device_->bytesToWrite();
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

void VersatileFile::ungetChar(char c)
{
	device_->ungetChar(c);
}

bool VersatileFile::putChar(char c)
{
	return device_->putChar(c);
}

bool VersatileFile::getChar(char* c)
{
	return device_->getChar(c);
}

QString VersatileFile::errorString() const
{
	return device_->errorString();
}

bool VersatileFile::waitForReadyRead(int msecs)
{
	return device_->waitForReadyRead(msecs);
}

bool VersatileFile::waitForBytesWritten(int msecs)
{
	return device_->waitForBytesWritten(msecs);
}

QByteArray VersatileFile::toByteArray()
{
	return readAll();
}

bool VersatileFile::atEnd() const
{
	return device_->atEnd();
}

bool VersatileFile::exists()
{
	return device_->isOpen();
}

qint64 VersatileFile::readLine(char* data, qint64 maxlen)
{
	return readLineData(data, maxlen);
}

QByteArray VersatileFile::readLine(qint64 maxlen)
{	
	return device_->readLine(maxlen);
}

QByteArray VersatileFile::readAll()
{
	checkIfOpen();
	return device_->readAll();
}

qint64 VersatileFile::read(char* data, qint64 maxlen)
{
	return readData(data, maxlen);
}

QByteArray VersatileFile::read(qint64 maxlen)
{
	checkIfOpen();
	return device_->read(maxlen);
}

bool VersatileFile::canReadLine() const
{
	checkIfOpen();
	return device_->canReadLine();
}

qint64 VersatileFile::readData(char* data, qint64 maxlen)
{
	checkIfOpen();
	return device_->read(data, maxlen);
}

qint64 VersatileFile::writeData(const char* data, qint64 len)
{
	checkIfOpen();
	return device_->write(data, len);
}

qint64 VersatileFile::readLineData(char* data, qint64 maxlen)
{
	checkIfOpen();
	return device_->readLine(data, maxlen);
}

void VersatileFile::checkIfOpen() const
{
	if (!device_->isOpen())
	{
		THROW(FileAccessException, "IODevice in VersatileFile is closed!");
	}
}
