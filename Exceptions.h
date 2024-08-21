#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include "cppCORE_global.h"
#include <QString>
#include <QDebug>

enum class ExceptionType
{
	CRITIAL, //a critical problem.
	WARNING, //a warning, that can be dealt with.
	INFO     //message is nothing out of the ordinary
};

///Base exception. Rather use derived exception if possible.
class CPPCORESHARED_EXPORT Exception
{
public:
	//Construtor.
	Exception(QString message, QString file, int line, ExceptionType type);

	//The error message.
	QString message() const;
	//The source code file the error occured in.
	QString file() const;
	//The source code line the error occured in.
	int line() const;

	//Returns the exception type.
	ExceptionType type() const;

protected:
	QString message_;
	QString file_;
	int line_;
	ExceptionType type_;
};

///Exception for dealing with server connection errors
class CPPCORESHARED_EXPORT HttpException
    : public Exception
{
public:
    //Construtor.
    HttpException(QString message, QString file, int line, ExceptionType type, int status_code, QMap<QByteArray, QByteArray> headers, QByteArray body);

    //Status code of HTTP response (e.g. 200 means OK, 404 - not found, 500 - internal server error, etc.)
    int status_code() const;
    //Headers of HTTP response
    QMap<QByteArray, QByteArray> headers();
    //Body of HTTP response
    QByteArray body() const;
    QString message() const;

protected:
    int status_code_;
    QMap<QByteArray, QByteArray> headers_;
    QByteArray body_;
    QString message_;
};

///Exception that is thrown when a function/method receives invalid arguments.
class CPPCORESHARED_EXPORT ArgumentException
		: public Exception
{
public:
	ArgumentException(QString message, QString file, int line, ExceptionType type);
};

///Exception that is thrown when files cannot be opened/written.
class CPPCORESHARED_EXPORT FileAccessException
		: public Exception
{
public:
	FileAccessException(QString message, QString file, int line, ExceptionType type);
};

///Exception that is thrown when file content cannot be parsed.
class CPPCORESHARED_EXPORT FileParseException
		: public Exception
{
public:
	FileParseException(QString message, QString file, int line, ExceptionType type);
};

///Exception for command line parsing errors of tools.
class CPPCORESHARED_EXPORT CommandLineParsingException
		: public Exception
{
public:
	CommandLineParsingException(QString message, QString file, int line, ExceptionType type);
};

///Exception that is thrown to indicate that the execution of a tool failed. It sets the error code to '1' and prints the message.
class CPPCORESHARED_EXPORT ToolFailedException
		: public Exception
{
public:
	ToolFailedException(QString message, QString file, int line, ExceptionType type);
};

///Exception for programming errors (forgotten switch cases, null pointers, etc).
class CPPCORESHARED_EXPORT ProgrammingException
		: public Exception
{
public:
	ProgrammingException(QString message, QString file, int line, ExceptionType type);
};

///Exception for database errors.
class CPPCORESHARED_EXPORT DatabaseException
		: public Exception
{
public:
	DatabaseException(QString message, QString file, int line, ExceptionType type);
};

///Exception for type conversion errors.
class CPPCORESHARED_EXPORT TypeConversionException
		: public Exception
{
public:
	TypeConversionException(QString message, QString file, int line, ExceptionType type);
};

///Exception for statistics errors.
class CPPCORESHARED_EXPORT StatisticsException
		: public Exception
{
public:
	StatisticsException(QString message, QString file, int line, ExceptionType type);
};

///Exception that indicates that the user aborted a dialog or action.
class CPPCORESHARED_EXPORT AbortByUserException
		: public Exception
{
public:
	AbortByUserException(QString message, QString file, int line, ExceptionType type);
};

///Exception that indicates that a certain functionality is not implemented.
class CPPCORESHARED_EXPORT NotImplementedException
		: public Exception
{
public:
	NotImplementedException(QString message, QString file, int line, ExceptionType type);
};

///Exception that indicates that a certain functionality is is not accessible by the current user.
class CPPCORESHARED_EXPORT AccessDeniedException
		: public Exception
{
public:
	AccessDeniedException(QString message, QString file, int line, ExceptionType type);
};

///Exception that indicates that some information is missing for a certain action.
class CPPCORESHARED_EXPORT InformationMissingException
		: public Exception
{
public:
	InformationMissingException(QString message, QString file, int line, ExceptionType type);
};

///Exception for Network errors
class CPPCORESHARED_EXPORT NetworkException
		: public Exception
{
public:
    NetworkException(QString message, QString file, int line, ExceptionType type);
};


#define THROW(name, message) \
throw name(message, __FILE__, __LINE__, ExceptionType::CRITIAL);

#define WARNING(name, message) \
throw name(message, __FILE__, __LINE__, ExceptionType::WARNING);

#define INFO(name, message) \
throw name(message, __FILE__, __LINE__, ExceptionType::INFO);


#define THROW_HTTP(name, message, status_code, headers, body) \
throw name(message, __FILE__, __LINE__, ExceptionType::CRITIAL, status_code, headers, body);

#endif // EXCEPTIONS_H
