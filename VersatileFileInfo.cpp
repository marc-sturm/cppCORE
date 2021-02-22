#include "VersatileFileInfo.h"

VersatileFileInfo::VersatileFileInfo(QString file)
	:file_(file)
{
	qDebug() << "Core versatile file info" << file;
	if (file.startsWith("http"))
	{
		file_info_provider_ = QSharedPointer<FileInfoProvider>(new FileInfoProviderRemote(file));
	}
	else
	{
		file_info_provider_ = QSharedPointer<FileInfoProvider>(new FileInfoProviderLocal(file));
	}
}

VersatileFileInfo::~VersatileFileInfo()
{
}

QString VersatileFileInfo::absolutePath()
{
	return file_info_provider_->absolutePath();
}

QString VersatileFileInfo::absoluteFilePath()
{
	return file_info_provider_->absoluteFilePath();
}

QString VersatileFileInfo::dirAbsolutePath()
{
	return file_info_provider_->dirAbsolutePath();
}

QString VersatileFileInfo::parentDirAbsolutePath()
{
	return file_info_provider_->parentDirAbsolutePath();
}

QString VersatileFileInfo::parentSubDirAbsolutePath(QString subdir)
{
	return file_info_provider_->parentSubDirAbsolutePath(subdir);
}

QString VersatileFileInfo::baseName()
{
	return file_info_provider_->baseName();
}

QString VersatileFileInfo::fileName()
{
	return file_info_provider_->fileName();
}

QString VersatileFileInfo::suffix()
{
	return file_info_provider_->suffix();
}

QDateTime VersatileFileInfo::lastModified()
{
	return file_info_provider_->lastModified();
}

bool VersatileFileInfo::exists()
{
	return file_info_provider_->exists();
}
