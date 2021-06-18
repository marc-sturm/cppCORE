#include "VersatileFile.h"
#include "HttpRequestHandler.h"

VersatileFile::VersatileFile(const QString& file_name)
	: file_name_(file_name)
{	
}

VersatileFile::~VersatileFile()
{	
	if (device_->isOpen()) device_->close();
}

bool VersatileFile::open(QIODevice::OpenMode mode)
{
	if (!file_name_.toLower().startsWith("http"))
	{
		// local file
		file_ = QSharedPointer<QFile>(new QFile(file_name_));
		file_.data()->open(mode);
		device_ = file_;
	}
	else
	{
		// remote file
		bool remote_file_accessed = false;
		try
		{
			reply_data_ = HttpRequestHandler(HttpRequestHandler::NONE).get(file_name_);
			remote_file_accessed = true;
		}
		catch (Exception& e)
		{
			qWarning() << "Could not get file location:" << e.message();
		}
		buffer_ = QSharedPointer<QBuffer>(new QBuffer(&reply_data_));
		if (remote_file_accessed) buffer_.data()->open(mode);

		device_ = buffer_;
	}

	return device_.data()->isOpen();
}

bool VersatileFile::open(FILE* f, QIODevice::OpenMode ioFlags)
{
	if (!file_name_.toLower().startsWith("http"))
	{
		file_ = QSharedPointer<QFile>(new QFile(file_name_));
		file_.data()->open(f, ioFlags);
		device_ = file_;
		return device_.data()->isOpen();
	}
	return false;
}

QIODevice::OpenMode VersatileFile::openMode() const
{
	return device_.data()->openMode();
}

bool VersatileFile::isOpen() const
{
	return device_.data()->isOpen();
}

bool VersatileFile::isReadable() const
{
	checkIfOpen();
	return device_.data()->isReadable();
}

qint64 VersatileFile::read(char* data, qint64 maxlen)
{
	checkIfOpen();
	return device_->read(data, maxlen);
}

QByteArray VersatileFile::read(qint64 maxlen)
{
	checkIfOpen();
	return device_.data()->read(maxlen);
}

QByteArray VersatileFile::readAll()
{
	checkIfOpen();
	return device_.data()->readAll();
}

qint64 VersatileFile::readLine(char* data, qint64 maxlen)
{
	checkIfOpen();
	return device_.data()->readLine(data, maxlen);
}

QByteArray VersatileFile::readLine(qint64 maxlen)
{
	checkIfOpen();
	return device_.data()->readLine(maxlen);
}

bool VersatileFile::canReadLine() const
{
	checkIfOpen();
	return device_.data()->canReadLine();
}

bool VersatileFile::atEnd() const
{
	checkIfOpen();
	return device_.data()->atEnd();
}

bool VersatileFile::exists()
{
	if (file_ != nullptr) return file_.data()->exists();
	if (buffer_ != nullptr) return buffer_.data()->data() != nullptr;
	return false;
}

void VersatileFile::close()
{
	if (device_->isOpen()) device_.data()->close();
}

bool VersatileFile::reset()
{
	checkIfOpen();
	return device_.data()->reset();
}

bool VersatileFile::isSequential() const
{
	checkIfOpen();
	return device_.data()->isSequential();
}

qint64 VersatileFile::pos() const
{
	checkIfOpen();
	return device_.data()->pos();
}

bool VersatileFile::seek(qint64 offset)
{
	checkIfOpen();
	return device_.data()->seek(offset);
}

qint64 VersatileFile::size() const
{
	checkIfOpen();
	return device_.data()->size();
}

QTextStream& VersatileFile::createTextStream()
{
	checkIfOpen();
	text_stream.setDevice(device_.data());
	return text_stream;
}

void VersatileFile::checkIfOpen() const
{
	if (!device_->isOpen())
	{
		THROW(FileAccessException, "IODevice in VersatileFile is closed!");
	}
}
