#ifndef GIT_H
#define GIT_H

#include <QString>
#include <QHash>
#include "cppCORE_global.h"

enum class GitStatus
{
	MODIFIED,
	ADDED,
	DELETED,
	NOT_VERSIONED
};

//Git helper class. Git CLI application has to be available via 'git_exe' in settings INI.
class CPPCORESHARED_EXPORT Git
{
public:
	//Returns if the directory is a git repository.
	static bool isRepo(QString dir);

	//Returns the 'git diff' status of the files in a repository. The returned file name is the canonical file path.
	static QHash<QString, GitStatus> status(QString dir);

	//Returns if there is something to pull
	static bool pullAvailable(QString dir);

	//Returns if there is something to push
	static bool pushAvailable(QString dir);

	//Perform 'git pull'
	static QByteArray pull(QString dir);

protected:
	static QString gitExe();

	Git() = delete;
};

#endif // GIT_H
