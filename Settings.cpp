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
	static QSharedPointer<QSettings> settings;
	if(settings.isNull())
	{
		QStringList default_paths = QStandardPaths::standardLocations(QStandardPaths::AppLocalDataLocation);
		if(default_paths.isEmpty()) THROW(Exception, "No local application data path was found!");
		QString path = default_paths[0];
		if (Helper::mkdir(path)==-1) THROW(Exception, "Could not create application data path '" + path + "'!");

		//set log file
		QString filename = path + QDir::separator() + QCoreApplication::applicationName() + "_local.ini";
		settings.reset(new QSettings(filename, QSettings::IniFormat));
		if (!settings->isWritable()) THROW(Exception, "Settings file '" + filename + "' is not writable!");
	}
	return *(settings.data());
}

const QSettings& Settings::settingsApplication()
{
	static QSettings* settings = nullptr;
	if(settings==nullptr)
	{
		QString filename = QCoreApplication::applicationDirPath() + QDir::separator() + QCoreApplication::applicationName() + ".ini";
		settings = new QSettings(filename, QSettings::IniFormat);
	}
	return *settings;
}

const QSettings& Settings::settingsGeneral()
{
	static QSettings* settings = nullptr;
	if(settings==nullptr)
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
	QSettings& settings = settingsApplicationUser();
	settings.setValue(key, value);
	settings.sync(); //sync, so that the file is created, which is checked by settingsApplicationUserExists
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
	QSettings& settings = settingsApplicationUser();
	settings.setValue(key, value);
	settings.sync(); //sync, so that the file is created, which is checked by settingsApplicationUserExists
}


QStringList Settings::stringList(QString key, bool optional)
{
	if (optional && !contains(key)) return QStringList();

	return valueWithFallback(key).toStringList();
}

void Settings::setStringList(QString key, QStringList value)
{
	QSettings& settings = settingsApplicationUser();
	settings.setValue(key, value);
	settings.sync(); //sync, so that the file is created, which is checked by settingsApplicationUserExists
}


bool Settings::boolean(QString key, bool optional)
{
	if (optional && !contains(key)) return false;

	return valueWithFallback(key).toBool();
}

void Settings::setBoolean(QString key, bool value)
{
	QSettings& settings = settingsApplicationUser();
	settings.setValue(key, value);
	settings.sync(); //sync, so that the file is created, which is checked by settingsApplicationUserExists
}

QMap<QString, QVariant> Settings::map(QString key, bool optional)
{
	if (optional && !contains(key)) QMap<QString, QVariant>();

	return valueWithFallback(key).toMap();
}

void Settings::setMap(QString key, QMap<QString, QVariant> value)
{
	QSettings& settings = settingsApplicationUser();
	settings.setValue(key, value);
	settings.sync(); //sync, so that the file is created, which is checked by settingsApplicationUserExists
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
	QVariant var = valueWithFallback(key);
	if (var.type()==QVariant::StringList) return var.toStringList().join("").trimmed()!=""; //special handling for QStringList
	if (var.type()==QVariant::Map) return var.toMap().keys().join("").trimmed()!=""; //special handling for QMap
	return var.toString().trimmed()!="";
}

QStringList Settings::files()
{
	QStringList output;
	if (settingsApplicationUserExists())
	{
		QString filename = settingsApplicationUser().fileName();
		if (QFile::exists(filename)) output << settingsApplicationUser().fileName();
	}

	QString filename = settingsApplication().fileName();
	if (QFile::exists(filename)) output << filename;
	filename = settingsGeneral().fileName();
	if (QFile::exists(filename)) output << filename;

	return output;
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
