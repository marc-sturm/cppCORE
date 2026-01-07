#ifndef HELPER_H
#define HELPER_H

#include "cppCORE_global.h"
#include "Exceptions.h"
#include <QElapsedTimer>
#include <QFile>
#include <QStringList>
#include <QSharedPointer>

///Auxilary helper functions class.
class CPPCORESHARED_EXPORT Helper
{
public:
	///Updates random number generator using a seed caclualted from the current time and process ID.
	static void randomInit();
	///Returns a random number in the given range
	static double randomNumber(double min, double max);
	///Returns a random string.
	static QString randomString(int length, const QString& chars="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
	///Returns the elapsed time as a human-readable string.
    static QByteArray elapsedTime(QElapsedTimer elapsed, bool only_seconds = false);
	///Returns the elapsed time as a human-readable string.
	static QByteArray elapsedTime(int elapsed_ms, bool only_seconds = false);

	///check if the argument is a int/float (also returns false in case of extra whitespaces)
	static bool isNumeric(QString str);

	///check if the argument is a int/float (also returns false in case of extra whitespaces)
	static bool isNumeric(QByteArray str);

	///Converts a QString/QByteArray to an integer. Throws an error if the conversion fails.
	template <typename T>
	static int toInt(const T& str, const QString& name = "string", const QString& line = "")
	{
		bool ok = false;
		int result = str.trimmed().toInt(&ok);
		if (!ok) THROW(ArgumentException, "Could not convert " + name + " '" + str + "' to integer" + (line.isEmpty() ? "" : "  - line: " + line));
		return result;
	}
	///Converts a QString/QByteArray to a double. Throws an error if the conversion fails.
	template <typename T>
	static double toDouble(const T& str, const QString& name = "string", const QString& line = "")
	{
		bool ok = false;
		double result = str.trimmed().toDouble(&ok);
		if (!ok) THROW(ArgumentException, "Could not convert " + name + " '" + str + "' to double" + (line.isEmpty() ? "" : "  - line: " + line));
		return result;
	}   

	///Returns an opened file pointer, or throws an error if it cannot be opened.
	static QSharedPointer<QFile> openFileForReading(QString file_name, bool stdin_if_empty=false);

	///Returns an opened file pointer, or throws an error if it cannot be opened.
	static QSharedPointer<QFile> openFileForWriting(QString file_name, bool stdout_if_file_empty=false, bool append=false);

	///Convenience overload for loadTextFile.
	static QStringList loadTextFile(QString file_name, bool trim_lines = false, QChar skip_header_char = QChar::Null, bool skip_empty_lines = false);
	///Stores a string list as a text file. '\r' and '\n' are trimmed from the end of each line and '\n' is appended as newline character.
	static void storeTextFile(QSharedPointer<QFile> file, const QStringList& lines);
	///Convenience overload for storeTextFile.
	static void storeTextFile(QString file_name, const QStringList& lines);

	///Returns the contents of a file as a string. Throws an error if the file cannot be opened.
	static QString fileText(QString filename);
	///Creates a text file if it does not exist. Throws an error if the file cannot be opened for writing.
	static void touchFile(QString filename);
	///Moves a file. Throws an error if something goes wrong.
	static void moveFile(QString from, QString to);

	///Returns a temporary file name. Make sure you delete the file when it is no longer needed to avoid name clashes!
	static QString tempFileName(QString extension, int length=16);
	///Returns a temporary file name following the schema [appname]_[appversion]_[suffix]. This is used to avoid several copies of the same file, e.g. when copying temporary files from resources.
	static QString tempFileNameNonRandom(QString suffix);

	///Find files (recursively).
	static QStringList findFiles(const QString& directory, const QString& pattern, bool recursive);
	///Find folders (recursively).
	static QStringList findFolders(const QString& directory, const QString& pattern, bool recursive);

	///Returns the Levenshtein-distance of two strings.
	static int levenshtein(const QString& s1, const QString& s2);

	///Gets the user name of the current user from the environment variables.
	static QString userName();

	///Returns the current date and time in the given format. If the format is a empty string, the ISO format "yyyy-MM-ddTHH:mm:ss" is returned.
	static QString dateTime(QString format = "dd.MM.yyyy hh:mm:ss");

	///Checks if a file is writable (or if the folder is writable in case the file does not exist)
	static bool isWritable(QString filename);

	///Retruns the canonical (relative or absolute) file path of the current OS.
	static QString canonicalPath(QString filename);

	///Removes all elements from a container that match the given predicate.
	template<typename T, typename TPredicate>
	static void removeIf(T& container, TPredicate predicate)
	{
		typename T::iterator it = std::remove_if(container.begin(), container.end(), predicate);
		container.erase(it, container.end());
	}

	///Trim all strings in a container
	template<typename T>
	static void trim(T& container)
	{
		for (int i=0; i<container.count(); ++i)
		{
			container[i] = container[i].trimmed();
		}
	}

	///Returns if the current OS is Windows.
	static bool isWindows();
	///Returns if the current OS is MacOS.
	static bool isMacOS();
	///Returns if the current OS is Linux.
	static bool isLinux();

	//Returns if the given filename is a HTTP/HTTPS URL.
	static bool isHttpUrl(QString filename);

	//Returns a reference to an empty QByteArray
	static const QByteArray& empty()
	{
		static QByteArray empty;
		return empty;
	}

	///Returns a formatted QString of a number
	static QString formatLargeNumber(long long input_number, const QString& format_type);

	///Creates the path if it does not exists. Returns 0 if the path already existed, 1 if it was created or -1 if it could not be created.
	static int mkdir(QString path);

	///Sets permissions to 777. Returns false if permissions could not be set.
	static bool set777(QString file);

	///Executes a command using the given arguments. Returns -1 if the execution failed and the exit code otherwise. If output is set, it is filled with the output from STDOUT and STDERR.
	static int executeCommand(QString command, QStringList args, QByteArrayList* output = nullptr);

	///Returns if the application is running in QtCreator
	static bool runningInQtCreator();

protected:
	///Constructor declared away.
	Helper() = delete;
};

#endif // HELPER_H
