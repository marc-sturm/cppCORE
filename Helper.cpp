#include "Helper.h"
#include "ToolBase.h"
#include "cmath"
#include "time.h"
#include <QDir>
#include <QCoreApplication>
#include <QDateTime>
#include <QCoreApplication>
#include <QProcess>

void Helper::randomInit()
{
	int seconds = time(NULL);
	int milliseconds = QTime::currentTime().msec();
	int process_id = QCoreApplication::applicationPid();
	srand(seconds + milliseconds + process_id + rand());
}

double Helper::randomNumber(double min, double max)
{
	double r = (double)rand() / (double)RAND_MAX;
	return min + r * (max - min);
}

QString Helper::randomString(int length, const QString& chars)
{
	//initialize random number generator once
	static bool is_initialized = false;
	if (!is_initialized)
	{
		randomInit();
		is_initialized = true;
	}

	//create random string
	QString output;
	for (int i=0; i<length; ++i)
	{
		int index = std::round(randomNumber(0, chars.length()-1));
		output.append(chars[index]);
	}
	return output;
}


QByteArray Helper::elapsedTime(QTime elapsed, bool only_seconds)
{
	return elapsedTime(elapsed.elapsed(), only_seconds);
}

QByteArray Helper::elapsedTime(int elapsed_ms, bool only_seconds)
{
	//calculate minutes and seconds
	double s = elapsed_ms/1000.0;
	double m = 0;
	double h = 0;
	if (!only_seconds)
	{
		m = floor(s/60.0);
		s -= 60.0 * m;
		h = floor(m/60.0);
		m -= 60.0 * h;
	}

	//create strings
	QByteArray sec = QByteArray::number(s, 'f', 3) + "s";
	QByteArray min = m==0.0 ? "" : QByteArray::number(m, 'f', 0) + "m ";
	QByteArray hours = h==0.0 ? "" : QByteArray::number(h, 'f', 0) + "h ";

	return hours + min + sec;
}

bool Helper::isNumeric(QString str)
{
	//remove minus
	if (str.startsWith('-')) str = str.mid(1);

	//check that we have digits and one dot max
	int c_digit = 0;
	int c_dot = 0;
	foreach(auto c, str)
	{
		if (c=='.') ++c_dot;
		else if (c.isDigit()) ++c_digit;
		else return false;
	}
	return c_digit>0 && c_dot<=1;
}

bool Helper::isNumeric(QByteArray str)
{
	//remove minus
	if (str.startsWith('-')) str = str.mid(1);

	//check that we have digits and one dot max
	int c_digit = 0;
	int c_dot = 0;
	foreach(auto c, str)
	{
		if (c=='.') ++c_dot;
		else if (isdigit(c)) ++c_digit;
		else return false;
	}
	return c_digit>0 && c_dot<=1;
}


QStringList Helper::loadTextFile(QString file_name, bool trim_lines, QChar skip_header_char, bool skip_empty_lines)
{
	QStringList output;
	VersatileTextStream stream(file_name);
	while (!stream.atEnd())
	{
		QString line = stream.readLine();

		//remove newline or trim
		if (trim_lines)
		{
			line = line.trimmed();
		}
		else
		{
			while (line.endsWith('\n') || line.endsWith('\r')) line.chop(1);
		}

		//skip empty lines
		if (skip_empty_lines && line.count()==0) continue;

		//skip header lines
		if (skip_header_char!=QChar::Null && line.count()!=0 && line[0]==skip_header_char) continue;

		output.append(line);
	}

	return output;
}

void Helper::storeTextFile(QSharedPointer<QFile> file, const QStringList& lines)
{
	QTextStream stream(file.data());
	foreach(QString line, lines)
	{
		while (line.endsWith('\n') || line.endsWith('\r')) line.chop(1);
		stream << line << "\n";
	}
}

void Helper::storeTextFile(QString file_name, const QStringList& lines)
{
	storeTextFile(openFileForWriting(file_name), lines);
}

QString Helper::fileText(QString filename)
{
	QFile file(filename);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		THROW(FileAccessException, "Could not open text file for reading: '" + filename + "'!");
	}

	QTextStream stream(&file); // we need a text stream to support UTF8 characters
	return stream.readAll();
}

void Helper::touchFile(QString filename)
{
	if (!QFile(filename).open(QFile::ReadWrite))
	{
		THROW(FileAccessException, "Could not open file for writing: '" + filename + "'!");
	}
}

void Helper::moveFile(QString from, QString to)
{
	if (QFile::exists(to) && !QFile(to).remove())
	{
		THROW(FileAccessException,"Could not remove file: " + to);
	}
	if (!QFile::rename(from, to))
	{
		THROW(FileAccessException,"Could not move file " + from + " to " + to + " !");
	}
}


QString Helper::tempFileName(QString extension, int length)
{
	QString name = Helper::randomString(length);
	if (extension!="")
	{
		if (!extension.startsWith(".")) name.append(".");
		name.append(extension);
	}

	return Helper::canonicalPath(QDir::tempPath() + "/" + name);
}

QString Helper::tempFileNameNonRandom(QString suffix)
{
	return Helper::canonicalPath(QDir::tempPath() + "/" + QCoreApplication::applicationName() + "_" + ToolBase::version() + "_" + suffix);
}

QStringList Helper::findFiles(const QString& directory, const QString& pattern, bool recursive)
{
	QStringList output;

	QDir dir(directory);
	if(!dir.exists()) THROW(FileAccessException, "Directory does not exist: " + directory);

	QFileInfoList file_infos = dir.entryInfoList(QStringList() << pattern, QDir::Files);
	foreach(const QFileInfo& entry, file_infos)
	{
		output.append(directory + "/" + entry.fileName());
	}

	if (recursive)
	{
		file_infos = dir.entryInfoList(QDir::AllDirs|QDir::NoDotAndDotDot);
		foreach(const QFileInfo& entry, file_infos)
		{
			output << findFiles(directory + "/" + entry.fileName(), pattern, true);
		}
	}

	return output;
}

QStringList Helper::findFolders(const QString& directory, const QString& pattern, bool recursive)
{
	QStringList output;

	QDir dir(directory);
	if(!dir.exists()) THROW(FileAccessException, "Directory does not exist: " + directory);

	QFileInfoList file_infos = dir.entryInfoList(QStringList() << pattern, QDir::Dirs|QDir::NoDotAndDotDot);
	foreach(const QFileInfo& entry, file_infos)
	{
		output.append(directory + "/" + entry.fileName());
	}
	if (recursive)
	{
		file_infos = dir.entryInfoList(QDir::AllDirs|QDir::NoDotAndDotDot);
		foreach(const QFileInfo& entry, file_infos)
		{
			output << findFolders(directory + "/" + entry.fileName(), pattern, true);
		}
	}

	return output;
}

int Helper::levenshtein(const QString& s1, const QString& s2)
{
	const int len1 = s1.size();
	const int len2 = s2.size();
	QVector<int> col(len2+1);
	QVector<int> prevCol(len2+1);

	for (int i=0; i<prevCol.size(); ++i)
	{
		prevCol[i] = i;
	}
	for (int i=0; i<len1; ++i)
	{
		col[0] = i+1;
		for (int j = 0; j < len2; j++)
		{
			col[j+1] = std::min( std::min(prevCol[1 + j] + 1, col[j] + 1), prevCol[j] + (s1[i]==s2[j] ? 0 : 1) );
		}
		col.swap(prevCol);
	}
	return prevCol[len2];
}

QString Helper::userName()
{
	return qgetenv(isWindows() ? "USERNAME" : "USER");
}

QString Helper::dateTime(QString format)
{
	if (format=="")
	{
		return QDateTime::currentDateTime().toString(Qt::ISODate);
	}

	return QDateTime::currentDateTime().toString(format);
}

bool Helper::isWritable(QString filename)
{
	if (QFile::exists(filename))
	{
		QFileInfo file_info(filename);
		if (!file_info.isFile() || !file_info.isWritable())
		{
			return false;
		}
	}
	else
	{
		QString dir = QFileInfo(filename).absolutePath();
		QFileInfo dir_info(dir);
		if (!dir_info.isDir()  || !dir_info.isWritable())
		{
			return false;
		}
	}

	return true;
}

QString Helper::canonicalPath(QString filename)
{
	if (Helper::isHttpUrl(filename)) return filename;

	//Use native separator for the current OS
	filename = QDir::toNativeSeparators(filename).trimmed();

	if (filename.isEmpty()) return filename;

	//double > single separator (except for first character in case of UNC path)
	QChar sep = QDir::separator();
	QString sep_twice = QString(sep)+sep;
	while(filename.mid(1).contains(sep_twice))
	{
		filename = filename.at(0) + filename.mid(1).replace(sep_twice, sep);
	}

	//remove "."
	QStringList parts = filename.split(sep);
	while (parts.contains("."))
	{
		int index = parts.indexOf(".");
		if (index==0) break;
		parts.removeAt(index);
	}

	//remove ".." and folder before it
	while (parts.contains(".."))
	{
		int index = parts.indexOf("..");
		if (index<=1) break;
		parts.removeAt(index);
		parts.removeAt(index-1);
	}

	return parts.join(sep);
}

bool Helper::isWindows()
{
	#ifdef _WIN32
		return true;
	#else
		return false;
	#endif
}

bool Helper::isMacOS()
{
	#ifdef __APPLE__
		return true;
	#else
		return false;
	#endif
}

bool Helper::isLinux()
{
	#ifdef __linux__
		return true;
	#else
		return false;
	#endif
}

QSharedPointer<QFile> Helper::openFileForReading(QString file_name, bool stdin_if_empty)
{
	QSharedPointer<QFile> file(new QFile(file_name));
	if (stdin_if_empty && file_name=="")
	{
		file->open(stdin, QFile::ReadOnly | QIODevice::Text);
	}
	else if (!file->open(QFile::ReadOnly | QIODevice::Text))
	{
		THROW(FileAccessException, "Could not open file for reading: '" + file_name + "'!");
	}
	return file;
}

QSharedPointer<VersatileFile> Helper::openVersatileFileForReading(QString file_name, bool stdin_if_empty)
{
	QSharedPointer<VersatileFile> file(new VersatileFile(file_name));
	if (stdin_if_empty && file_name=="")
	{
		file->open(stdin, QFile::ReadOnly | QIODevice::Text);
	}
	else if (!file->open(QFile::ReadOnly | QIODevice::Text))
	{
		THROW(FileAccessException, "Could not open file for reading: '" + file_name + "'!");
	}

	return file;
}

QSharedPointer<QFile> Helper::openFileForWriting(QString file_name, bool stdout_if_file_empty, bool append)
{
	QSharedPointer<QFile> file(new QFile(file_name));
	if (stdout_if_file_empty && file_name=="")
	{
		file->open(stdout, QFile::WriteOnly | QIODevice::Text);
	}
	else if (!file->open(QFile::WriteOnly | QIODevice::Text |(append? QFile::Append : QFile::Truncate)))
	{
		THROW(FileAccessException, "Could not open file for writing: '" + file_name + "'!");
	}

	return file;
}

bool Helper::isHttpUrl(QString filename)
{
	filename = filename.trimmed().toLower();
	return filename.startsWith("http:") || filename.startsWith("https:");
}

QString Helper::formatLargeNumber(long long input_number, const QString& format_type)
{
	if (format_type == "raw_counts")
	{
		return QString::number(input_number);
	}
	else if (format_type == "modifier")
	{
		if (input_number >= 1e13) return QString::number(input_number/1e12, 'f', 2) + " T";
		if (input_number >= 1e10) return QString::number(input_number/1e9, 'f', 2) + " G";
		if (input_number >= 1e7) return QString::number(input_number/1e6, 'f', 2) + " M";
		if (input_number >= 1e4) return QString::number(input_number/1e3, 'f', 2) + " k";
		return QString::number(input_number);
	}
	else if (format_type == "thousands_separator")
	{
		QLocale english(QLocale::English, QLocale::UnitedStates);
		return english.toString(input_number);
	}
	THROW(ArgumentException, "Invalid format type '" + format_type + "' provided. \n(Valid types are: 'raw_counts', 'modifier', 'thousands_separator')")
}

int Helper::mkdir(QString path)
{
	QDir dir(path);
	if (dir.exists()) return 0;

	return dir.mkpath(".") ? 1 : -1;
}

bool Helper::set777(QString file)
{
	return QFile::setPermissions(file, QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner | QFileDevice::ReadGroup | QFileDevice::WriteGroup | QFileDevice::ExeGroup | QFileDevice::ReadOther | QFileDevice::WriteOther | QFileDevice::ExeOther);
}

int Helper::executeCommand(QString command, QStringList args, QByteArrayList* output)
{
	QProcess process;
	process.setProcessChannelMode(QProcess::MergedChannels);
	process.start(command + " " + args.join(' '));
	bool success = process.waitForFinished(-1);

	//set output
	if (output!=nullptr)
	{
		output->clear();
		output->operator=(process.readAll().split('\n'));
	}

	//special case exit code handling
	if (!success || process.exitStatus()!=QProcess::NormalExit) return -1;

	return process.exitCode();
}
