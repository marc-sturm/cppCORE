#ifndef VERSATILEFILEINFO_H
#define VERSATILEFILEINFO_H

#include "cppCORE_global.h"
#include "FileInfoProvider.h"
#include "FileInfoProviderLocal.h"
#include "FileInfoProviderRemote.h"

class CPPCORESHARED_EXPORT VersatileFileInfo
{
public:
	VersatileFileInfo(QString file);
	~VersatileFileInfo();

	QString absolutePath();
	QString absoluteFilePath();
	QString dirAbsolutePath();
	QString parentDirAbsolutePath();
	QString parentSubDirAbsolutePath(QString subdir);
	QString baseName();
	QString fileName();
	QString suffix();
	QDateTime lastModified();
	bool exists();

private:
	QString file_;
	QSharedPointer<FileInfoProvider> file_info_provider_;
};

#endif // VERSATILEFILEINFO_H
