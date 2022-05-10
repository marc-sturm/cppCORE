#include "Settings.h"
#include <QDir>
#include <QStandardPaths>
#include "SimpleCrypt.h"
#include "Log.h"
#include "ToolBase.h"
#include "Exceptions.h"
#include "Helper.h"


bool Settings::settingsApplicationUserExists()
{
	QStringList default_paths = QStandardPaths::standardLocations(QStandardPaths::AppLocalDataLocation);
	if(default_paths.isEmpty()) return false;

	QString filename = default_paths[0] + QDir::separator() + QCoreApplication::applicationName() + "_local.ini";
	if (!QFile::exists(filename)) return false;

	return true;
}

QSettings& Settings::settingsApplicationUser()
{
	static QSettings* settings = 0;
	if(settings==0)
	{
		QStringList default_paths = QStandardPaths::standardLocations(QStandardPaths::AppLocalDataLocation);
		if(default_paths.isEmpty()) THROW(Exception, "No local application data path was found!");
		QString path = default_paths[0];
		if (!QDir().mkpath(path)) THROW(Exception, "Could not create application data path '" + path + "'!");

		//set log file
		QString filename = path + QDir::separator() + QCoreApplication::applicationName() + "_local.ini";
		settings = new QSettings(filename, QSettings::IniFormat);
	}
	return *settings;
}

const QSettings& Settings::settingsApplication()
{
	static QSettings* settings = 0;
	if(settings==0)
	{
		QString filename = QCoreApplication::applicationDirPath() + QDir::separator() + QCoreApplication::applicationName() + ".ini";
		settings = new QSettings(filename, QSettings::IniFormat);
	}
	return *settings;
}

const QSettings& Settings::settingsGeneral()
{
	static QSettings* settings = 0;
	if(settings==0)
	{
		QString filename = QCoreApplication::applicationDirPath() + QDir::separator() + "settings.ini";
		settings = new QSettings(filename, QSettings::IniFormat);
	}
	return *settings;
}

int Settings::integer(QString key)
{
	return valueWithFallback(key).toInt();
}

void Settings::setInteger(QString key, int value)
{
	settingsApplicationUser().setValue(key, value);
}

QString Settings::string(QString key, bool optional)
{
	if (optional && !contains(key)) return "";

	QString value = valueWithFallback(key).toString();

	//decrypt if encrypted
	QString crypt_prefix = "encrypted:";
	if (value.startsWith(crypt_prefix))
	{
		//remove prefix
		value = value.mid(crypt_prefix.count()).trimmed();

		//decrypt
		qulonglong crypt_key = ToolBase::encryptionKey("setting entry '" + key + "'");
		value = SimpleCrypt(crypt_key).decryptToString(value);
	}

	return value;
}

void Settings::setString(QString key, QString value)
{
	settingsApplicationUser().setValue(key, value);
}


QStringList Settings::stringList(QString key, bool optional)
{
	if (optional && !contains(key)) return QStringList();

	return valueWithFallback(key).toStringList();
}

void Settings::setStringList(QString key, QStringList value)
{
	settingsApplicationUser().setValue(key, value);
}


bool Settings::boolean(QString key, bool optional)
{
	if (optional && !contains(key)) return false;

	return valueWithFallback(key).toBool();
}

void Settings::setBoolean(QString key, bool value)
{
	settingsApplicationUser().setValue(key, value);
}

QMap<QString, QVariant> Settings::map(QString key, bool optional)
{
	if (optional && !contains(key)) QMap<QString, QVariant>();

	return valueWithFallback(key).toMap();
}

void Settings::setMap(QString key, QMap<QString, QVariant> value)
{
	settingsApplicationUser().setValue(key, value);
}

QString Settings::path(QString key, bool optional)
{
	if (optional && !contains(key)) return "";

	QString path = string(key).trimmed();

	//convert separators
	path = Helper::canonicalPath(path);

	//add separator at the end if missing from path
	if (QFile::exists(path) && QFileInfo(path).isDir() && !path.endsWith(QDir::separator()))
	{
		path += QDir::separator();
	}

	return path;
}

void Settings::setPath(QString key, QString path)
{
	QFileInfo info(path);
	if (info.isFile())
	{
		path = info.absolutePath();
	}

	if (QDir(path).exists())
	{
		setString(key, Helper::canonicalPath(path));
	}
}

void Settings::clear()
{
	settingsApplicationUser().clear();
}

void Settings::remove(QString key)
{
	settingsApplicationUser().remove(key);
}

QStringList Settings::allKeys()
{
	QStringList output;

	if (settingsApplicationUserExists())
	{
		output << settingsApplicationUser().allKeys();
	}
	output << settingsApplication().allKeys();
	output << settingsGeneral().allKeys();

	std::sort(output.begin(), output.end());
	output.removeDuplicates();

	return output;
}

bool Settings::contains(QString key)
{
	//check if key exists
	bool key_exists = (settingsApplicationUserExists() && settingsApplicationUser().contains(key)) || settingsApplication().contains(key) || settingsGeneral().contains(key);
	if (!key_exists) return false;

	//if the key exists, check that the value is not empty
	return valueWithFallback(key).toString().trimmed()!="";
}

QVariant Settings::valueWithFallback(QString key)
{
	if (settingsApplicationUserExists() && settingsApplicationUser().contains(key))
	{
		return settingsApplicationUser().value(key);
	}

	if (settingsApplication().contains(key))
	{
		return settingsApplication().value(key);
	}

	if (settingsGeneral().contains(key))
	{
		return settingsGeneral().value(key);
	}

	THROW(ProgrammingException, "Requested key '" + key + "' not found in settings!");
}
