#include "LoggingWorker.h"
#include <QCoreApplication>
#include "Helper.h"

LoggingWorker::LoggingWorker(QString file_name, QString message, QString level_str)
    : QRunnable()
    , file_name_(file_name)
    , message_(message)
    , level_str_(level_str)
{
}

void LoggingWorker::run()
{
    QString timestamp = Helper::dateTime("yyyy-MM-ddThh:mm:ss.zzz");
    QString message_sanitized = QString(message_).replace("\t", " ").replace("\n", "");

    QSharedPointer<QFile> out_file = Helper::openFileForWriting(file_name_, false, true);
    out_file->write((timestamp + "\t" + QString::number(QCoreApplication::applicationPid()) + "\t" + level_str_ + "\t" + message_sanitized).toUtf8() + "\n");
    out_file->flush();
}
