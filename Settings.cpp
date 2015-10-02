#include "Settings.h"
#include <QDir>
#include <QDebug>
#include "Log.h"

QSettings& Settings::settings()
{
	static QSettings* settings = 0;
	if(settings==0)
	{
		QString filename = QCoreApplication::applicationDirPath() + QDir::separator() + QCoreApplication::applicationName().replace(".exe","") + ".ini";
		settings = new QSettings(filename, QSettings::IniFormat);
	}
	return *settings;
}

QSettings& Settings::settingsFallback()
{
	static QSettings* settings = 0;
	if(settings==0)
	{
		QString filename = QCoreApplication::applicationDirPath() + QDir::separator() + "settings.ini";
		settings = new QSettings(filename, QSettings::IniFormat);
	}
	return *settings;
}

int Settings::integer(QString key, int default_value)
{
  int value = valueWithFallback(key, default_value).toInt();
  if (value==default_value)
  {
    setInteger(key, default_value);
  }
  return value;
}

void Settings::setInteger(QString key, int value)
{
  settings().setValue(key, value);
}

QString Settings::string(QString key, QString default_value)
{
  QString value = valueWithFallback(key, default_value).toString();
  if (value==default_value)
  {
    setString(key, default_value);
  }
  return value;
}

void Settings::setString(QString key, QString value)
{
  settings().setValue(key, value);
}


QStringList Settings::stringList(QString key, QStringList default_value)
{
  QStringList value = valueWithFallback(key, default_value).toStringList();
  if (value==default_value)
  {
    setStringList(key, default_value);
  }
  return value;
}

void Settings::setStringList(QString key, QStringList value)
{
  settings().setValue(key, value);
}


bool Settings::boolean(QString key, bool default_value)
{
  bool value = valueWithFallback(key, default_value).toBool();
  if (value==default_value)
  {
    setBoolean(key, default_value);
  }
  return value;
}

void Settings::setBoolean(QString key, bool value)
{
  settings().setValue(key, value);
}

QMap<QString, QVariant> Settings::map(QString key, QMap<QString, QVariant> default_value)
{
  QMap<QString, QVariant> value = valueWithFallback(key, default_value).toMap();
  if (value==default_value)
  {
    setMap(key, default_value);
  }
  return value;
}

void Settings::setMap(QString key, QMap<QString, QVariant> value)
{
  settings().setValue(key, value);
}

QString Settings::path(QString key, QString default_value)
{
  QString path = string(key, default_value);

  if (path==default_value || !QDir(path).exists())
  {
    setPath(key, default_value);
    return default_value;
  }

  if (!path.endsWith("/"))
  {
    path += "/";
  }

  return QDir::toNativeSeparators(path);
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
    setString(key, QDir::toNativeSeparators(path));
  }
}

void Settings::clear()
{
    settings().clear();
}

void Settings::remove(QString key)
{
	settings().remove(key);
}

QStringList Settings::allKeys()
{
	return settings().allKeys();
}

QString Settings::fileName()
{
	return settings().fileName();
}

void Settings::createBackup(QString suffix)
{
	QSettings backup(fileName() + suffix, QSettings::IniFormat);

	backup.clear();
	QStringList keys = allKeys();
	foreach (QString key, keys)
	{
		backup.setValue(key, settings().value(key));
	}
}

void Settings::restoreBackup(QString suffix)
{
	QSettings backup(fileName() + suffix, QSettings::IniFormat);

	clear();
	QStringList keys = backup.allKeys();
	foreach (QString key, keys)
	{
		settings().setValue(key, backup.value(key));
	}

	Log::info("Restored INI file: " + fileName() + suffix);
}

QVariant Settings::valueWithFallback(const QString& key, const QVariant& defaultValue)
{
	QVariant output = settings().value(key, defaultValue);

	if (output==defaultValue)
	{
		output = settingsFallback().value(key, defaultValue);
	}

	return output;
}
