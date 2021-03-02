#ifndef FILEINFOPROVIDER_H
#define FILEINFOPROVIDER_H

#include "cppCORE_global.h"
#include "Exceptions.h"
#include <QFileInfo>

class CPPCORESHARED_EXPORT FileInfoProvider
{
public:
	virtual ~FileInfoProvider(){}

	virtual QString absolutePath() = 0;
	virtual QString absoluteFilePath() = 0;
	virtual QString dirAbsolutePath() = 0;
	virtual QString parentDirAbsolutePath() = 0;
	virtual QString parentSubDirAbsolutePath(QString subdir) = 0;
	virtual bool exists() = 0;
	virtual QString baseName() = 0;
	virtual QString fileName() = 0;
	virtual QString suffix() = 0;
	virtual QDateTime lastModified() = 0;
};


#endif // FILEINFOPROVIDER_H
