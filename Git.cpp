#include "Git.h"
#include "Settings.h"
#include "Exceptions.h"
#include <QFileInfo>

bool Git::isRepo(QString dir)
{
	return QFile::exists(dir + "/.git");
}

QHash<QString, GitStatus> Git::status(QString dir)
{
	//execute
	QProcess process;
	process.setProcessChannelMode(QProcess::MergedChannels);
	process.start(gitExe(), QStringList() << "-C"  << dir << "status" << "--porcelain");
	if (!process.waitForFinished(-1)) throwException(process);
	QStringList text = QString(process.readAll()).split("\n");

	//convert git status lines to hash
	QHash<QString, GitStatus> output;
	foreach(QString line, text)
	{
		line = line.trimmed();
		if (line.isEmpty()) continue;

		line = line.replace("  ", " ").replace("\"", ""); //handle escaping of files with spaces
		int pos = line.indexOf(' ');
		if (pos==-1) continue;

		//convert status to enum
		QString status = line.left(pos);
		GitStatus status_enum = GitStatus::NOT_VERSIONED;
		if (status=="M")
		{
			status_enum = GitStatus::MODIFIED;
		}
		else if (status=="D")
		{
			status_enum = GitStatus::DELETED;
		}
		else if (status=="A" || status=="AM")
		{
			status_enum = GitStatus::ADDED;
		}
		else if (status=="??")
		{
			status_enum = GitStatus::NOT_VERSIONED;
		}
		else
		{
			THROW(FileParseException, "Git status '" + status + "' not known!");
		}

		QString filename = dir + "/" + line.mid(pos+1);
		filename = filename.replace("//", "/"); //remove duplicated slashes
		if (status_enum!=GitStatus::DELETED) filename = QFileInfo(filename).canonicalFilePath();
		output[filename] = status_enum;
	}

	return output;
}

bool Git::pullAvailable(QString dir)
{
	QString git_exe = gitExe();

	//update remote info
	QProcess process;
	process.setProcessChannelMode(QProcess::MergedChannels);
	process.start(git_exe, QStringList() << "-C"  << dir << "remote" << "update");
	if (!process.waitForFinished(-1)) throwException(process);
	QByteArrayList output = process.readAll().split('\n');

	//get status
	process.start(git_exe, QStringList() << "-C"  << dir << "status" << "-u" << "no");
	if (!process.waitForFinished(-1)) throwException(process);
	output = process.readAll().split('\n');

	//check status
	foreach(const QByteArray& line, output)
	{
		if (line.contains("Your branch is behind") && line.contains("can be fast-forwarded")) return true;
	}
	return false;
}

bool Git::pushAvailable(QString dir)
{
	//get status
	QProcess process;
	process.start(gitExe(), QStringList() << "-C"  << dir << "status" << "-u" << "no");
	if (!process.waitForFinished(-1)) throwException(process);
	QByteArrayList output = process.readAll().split('\n');

	//check status
	foreach(const QByteArray& line, output)
	{
		if (line.contains("Your branch is ahead of")) return true;
	}
	return false;
}

QByteArray Git::pull(QString dir)
{
	QString git_exe = gitExe();

	QProcess process;
	process.setProcessChannelMode(QProcess::MergedChannels);
	process.setWorkingDirectory(dir);
	process.start(git_exe, QStringList() << "pull" << "--recurse-submodules");
	if (!process.waitForFinished(-1)) throwException(process);
	return process.readAll();
}

QByteArray Git::push(QString dir)
{
	QString git_exe = gitExe();

	QProcess process;
	process.setProcessChannelMode(QProcess::MergedChannels);
	process.setWorkingDirectory(dir);
	process.start(git_exe, QStringList() << "push");
	if (!process.waitForFinished(-1)) throwException(process);
	return process.readAll();
}

QByteArray Git::branch(QString dir)
{
	QString git_exe = gitExe();

	QProcess process;
	process.setProcessChannelMode(QProcess::MergedChannels);
	process.setWorkingDirectory(dir);
	process.start(git_exe, QStringList() << "branch");
	if (!process.waitForFinished(-1)) throwException(process);

	QByteArrayList lines = process.readAll().split('\n');
	foreach(QByteArray line, lines)
	{
		line = line.trimmed();
		if (line.startsWith("* "))
		{
			QByteArray branch = line.mid(2);
			if (branch=="master" || branch=="main") continue;

			return branch;
		}
	}

	return "";
}

QString Git::gitExe()
{
	QString exe = Settings::string("git_exe", true);
	if (exe=="") THROW(InformationMissingException, "Git executable not found in settings!");

	return exe;
}

void Git::throwException(QProcess& process)
{
	THROW(Exception, "Could not execute '"+process.program() + " " + process.arguments().join(" ")+"'\n\nERROR:\n" + process.errorString() + "\nOutput:\n"+process.readAll());
}
