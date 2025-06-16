#include "Log.h"
#include "Exceptions.h"
#include "Helper.h"
#include <QTextStream>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QDir>

Log::Log()
	: log_cmd_(true)
	, log_file_(false)
	, log_file_name_()
	, enabled_(LOG_PERFORMANCE|LOG_INFO|LOG_WARNING|LOG_ERROR)
    , thread_pool_()
{
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
    inst().log_file_name_mutex_.lock();
    //use default log file location if unset
	if (inst().log_file_name_=="")
	{
		//determine and create data path
		QStringList default_paths = QStandardPaths::standardLocations(QStandardPaths::AppLocalDataLocation);
		if(default_paths.isEmpty()) THROW(Exception, "No local application data path was found!");
		QString path = default_paths[0];
		if (Helper::mkdir(path)==-1) THROW(Exception, "Could not create application data path '" + path + "'!");

		//set log file
		inst().log_file_name_ = path + QDir::separator() + QCoreApplication::applicationName() + ".log";
	}
    inst().log_file_name_mutex_.unlock();
	inst().log_file_ = enabled;
}

void Log::setFileName(QString filename)
{
    inst().log_file_name_mutex_.lock();
    inst().log_file_name_ = filename;
    inst().log_file_name_mutex_.unlock();
	inst().log_file_ = true;
    inst().thread_pool_.setMaxThreadCount(1);
    inst().thread_pool_.setExpiryTimeout(30000);
}

QString Log::fileName()
{
    inst().log_file_name_mutex_.lock();
    QString current_file_name = inst().log_file_name_;
    inst().log_file_name_mutex_.unlock();
    return current_file_name;
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

void Log::perf(const QString& message, QElapsedTimer elapsed)
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
            stream << level_str << ": " << message << QT_ENDL;
		}
		else
		{
			QTextStream stream(stdout);
            stream << level_str << ": " << message << QT_ENDL;
		}
	}
	//FILE
	if (log_file_)
	{        
		try
		{
            LoggingWorker* logging_worker = new LoggingWorker(fileName(), message, level_str);
            inst().thread_pool_.start(logging_worker);
		}
		catch(Exception& e)
		{
			QTextStream stream(stderr);
            stream << levelString(LOG_ERROR) << ": Could not write to log file " << fileName() << ": " << e.message() << QT_ENDL;

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
