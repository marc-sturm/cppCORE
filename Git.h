#ifndef GIT_H
#define GIT_H

#include <QString>
#include <QHash>
#include <QProcess>
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

	//Perform 'git push'
	static QByteArray push(QString dir);

    //Perform 'git stash'
    static QByteArray stash(QString dir);

    //Perform 'git stash pop'
    static QByteArray stashPop(QString dir);

    //Returns the branch the repository is on (if not master/main)
	static QByteArray branch(QString dir);

    //Sets the Git exe override
    static void setGitOverride(QString exe);

protected:
    //Override for git executable (in case settings are not available).
    static QString git_exe_override;
    //Returns the GIT exe from the Settings or override.
    static QString gitExe();
    //Throws an exception after a command has failed
	static void throwException(QProcess& process);

	Git() = delete;
};

#endif // GIT_H
