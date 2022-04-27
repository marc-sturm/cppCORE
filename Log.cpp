#include "Log.h"
#include "Exceptions.h"
#include "Helper.h"
#include <QTextStream>
#include <QCoreApplication>
#include <QFile>
#include <QStandardPaths>
#include <QDir>

Log::Log()
	: log_cmd_(true)
	, log_file_(false)
	, log_file_name_()
	, enabled_(LOG_PERFORMANCE|LOG_INFO|LOG_WARNING|LOG_ERROR)
{
	//determine and create data path
	QStringList default_paths = QStandardPaths::standardLocations(QStandardPaths::AppLocalDataLocation);
	if(default_paths.isEmpty()) THROW(Exception, "No local application data path was found!");
	QString path = default_paths[0];
	if (!QDir().mkpath(path)) THROW(Exception, "Could not create application data path '" + path + "'!");

	//set log file
	log_file_name_ = path + QDir::separator() + QCoreApplication::applicationName() + ".log";
}

Log& Log::inst()
{
	static Log logger;
	return logger;
}

void Log::setCMDEnabled(bool enabled)
{
	inst().log_cmd_ = enabled;
}

void Log::setFileEnabled(bool enabled)
{
	inst().log_file_ = enabled;
}

void Log::setFileName(QString filename)
{
	inst().log_file_name_ = filename;
	inst().log_file_ = true;
}

QString Log::fileName()
{
	return inst().log_file_name_;
}

void Log::enableLogLevels(int enabled)
{
	inst().enabled_ = enabled;
}

void Log::error(const QString& message)
{
	inst().logMessage(LOG_ERROR, message);
}

void Log::warn(const QString& message)
{
	inst().logMessage(LOG_WARNING, message);
}

void Log::info(const QString& message)
{
	inst().logMessage(LOG_INFO, message);
}

void Log::perf(const QString& message)
{
	inst().logMessage(LOG_PERFORMANCE, message);
}

void Log::perf(const QString& message, QTime elapsed)
{
	inst().logMessage(LOG_PERFORMANCE, message.trimmed() + " " + Helper::elapsedTime(elapsed));
}

void Log::appInfo()
{
	inst().logMessage(LOG_INFO, "Application path: " + QCoreApplication::applicationFilePath());
	inst().logMessage(LOG_INFO, "Application version: " + QCoreApplication::applicationVersion());
	inst().logMessage(LOG_INFO, "User: " + Helper::userName());
}

void Log::logMessage(LogLevel level, const QString& message)
{
	if (!(enabled_&level)) return;
	QString level_str = levelString(level);

	//CMD
	if (log_cmd_)
	{
		if (level==LOG_ERROR || level==LOG_WARNING)
		{
			QTextStream stream(stderr);
			stream << level_str << ": " << message << endl;
		}
		else
		{
			QTextStream stream(stdout);
			stream << level_str << ": " << message << endl;
		}
	}
	//FILE
	if (log_file_)
	{

		QString timestamp = Helper::dateTime("yyyy-MM-ddThh:mm:ss.zzz");
		QString name = QCoreApplication::applicationName().replace(".exe", "");
		QString message_sanitized = QString(message).replace("\t", " ").replace("\n", "");

		try
		{
			QSharedPointer<QFile> out_file = Helper::openFileForWriting(log_file_name_, false, true);
			out_file->write((timestamp + "\t" + name + "\t" + level_str + "\t" + message_sanitized).toLatin1() + "\n");
			out_file->flush();
		}
		catch(Exception& e)
		{
			QTextStream stream(stderr);
			stream << levelString(LOG_ERROR) << ": Could not write to log file " << log_file_name_ << ": " << e.message() << endl;

			throw e;
		}
	}
}

QString Log::levelString(Log::LogLevel level)
{
	switch(level)
	{
		case LOG_PERFORMANCE:
			return "PERFORMANCE";
			break;
		case LOG_INFO:
			return "INFO";
			break;
		case LOG_WARNING:
			return "WARNING";
			break;
		case LOG_ERROR:
			return "ERROR";
			break;
	}

	THROW(ProgrammingException, "Unknown log level '" + QString::number(level) + "'!");
}
