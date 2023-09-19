#include "Exceptions.h"

Exception::Exception(QString message, QString file, int line, ExceptionType type)
	: message_(message)
	, file_(file)
	, line_(line)
	, type_(type)
{
}

QString Exception::message() const
{
	return message_;
}

QString Exception::file() const
{
	return file_;
}

int Exception::line() const
{
	return line_;
}

ExceptionType Exception::type() const
{
	return type_;
}

ArgumentException::ArgumentException(QString message, QString file, int line, ExceptionType type)
	: Exception(message, file, line, type)
{
}

FileAccessException::FileAccessException(QString message, QString file, int line, ExceptionType type)
	: Exception(message, file, line, type)
{
}

FileParseException::FileParseException(QString message, QString file, int line, ExceptionType type)
	: Exception(message, file, line, type)
{
}

ToolFailedException::ToolFailedException(QString message, QString file, int line, ExceptionType type)
	: Exception(message, file, line, type)
{
}
CommandLineParsingException::CommandLineParsingException(QString message, QString file, int line, ExceptionType type)
	: Exception(message, file, line, type)
{
}

ProgrammingException::ProgrammingException(QString message, QString file, int line, ExceptionType type)
	: Exception(message, file, line, type)
{
}

DatabaseException::DatabaseException(QString message, QString file, int line, ExceptionType type)
	: Exception(message, file, line, type)
{
}

TypeConversionException::TypeConversionException(QString message, QString file, int line, ExceptionType type)
	: Exception(message, file, line, type)
{
}

StatisticsException::StatisticsException(QString message, QString file, int line, ExceptionType type)
	: Exception(message, file, line, type)
{
}

AbortByUserException::AbortByUserException(QString message, QString file, int line, ExceptionType type)
	: Exception(message, file, line, type)
{
}

NotImplementedException::NotImplementedException(QString message, QString file, int line, ExceptionType type)
	: Exception(message, file, line, type)
{
}

AccessDeniedException::AccessDeniedException(QString message, QString file, int line, ExceptionType type)
	: Exception(message, file, line, type)
{
}

InformationMissingException::InformationMissingException(QString message, QString file, int line, ExceptionType type)
	: Exception(message, file, line, type)
{
}

NetworkException::NetworkException(QString message, QString file, int line, ExceptionType type)
	: Exception(message, file, line, type)
{

}

HttpException::HttpException(QString message, QString file, int line, ExceptionType type, int status_code, QMap<QByteArray, QByteArray> headers, QByteArray body)
    : Exception(message, file, line, type)
    , status_code_(status_code)
    , headers_(headers)
    , body_(body)
{
}

int HttpException::status_code() const
{
    return status_code_;
}

QMap<QByteArray, QByteArray> HttpException::headers()
{
    return headers_;
}

QByteArray HttpException::body() const
{
    return body_;
}
