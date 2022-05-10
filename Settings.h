#ifndef SETTINGS_H
#define SETTINGS_H

#include "cppCORE_global.h"
#include <QSettings>
#include <QStringList>
#include <QCoreApplication>


///Application settings handler
class CPPCORESHARED_EXPORT Settings
{
  public:
	///Read access for integer settings.
	static int integer(QString key);
	///Write access for integer settings.
	static void setInteger(QString key, int value);

	///Read access for string settings. If the string starts with 'encrypted:' it is decrypted before returning the key. If optional and not present, an empty string is returned.
	static QString string(QString key, bool optional=false);
	///Write access for string settings.
	static void setString(QString key, QString value);

	///Read access for string list settings. If optional and not present, an empty string list is returned.
	static QStringList stringList(QString key, bool optional=false);
	///Write access for string list settings.
	static void setStringList(QString key, QStringList value);

	///Read access for boolean settings. If optional and not present, 'false' is retured.
	static bool boolean(QString key, bool optional=false);
	///Write access for boolean settings.
	static void setBoolean(QString key, bool value);

	///Read access for map settings. If optional and not present, an empty map is returned.
	static QMap<QString,QVariant> map(QString key, bool optional=false);
	///Write access for map settings.
	static void setMap(QString key, QMap<QString,QVariant> value);

	///Read access for file or directory path settings. If optional and not present, an empty string is returned.
	static QString path(QString key, bool optional=false);
	///Write access for file or directory path settings. If the path contains a file name, it is stripped off.
    static void setPath(QString key, QString path);

	///Removes all settings from the user-sepecific settings file.
    static void clear();
	///Removes an entry from the user-sepecific settings file.
    static void remove(QString key);

	///Returns all available keys.
	static QStringList allKeys();
	///Returns if a key is present and the value is not empty.
	static bool contains(QString key);

  protected:
	///Default constructor "declared away"
	Settings() = delete;

	///Retreives a value from the settings, with fallback in this order: settingsApplicationUser(), settingsApplication(), settingsGeneral().
	static QVariant valueWithFallback(QString key);

	static bool settingsApplicationUserExists();
	///returns the user-specific settings file "[appname].ini" in the local application data folder (read-write)
	static QSettings& settingsApplicationUser();
	///returns the application-specific settings file "[appname].ini" in the application folder (read only)
	static const QSettings& settingsApplication();
	///returns the general settings file "settings.ini" in the application folder (read only)
	static const QSettings& settingsGeneral();
};

#endif //SETTINGS_H
