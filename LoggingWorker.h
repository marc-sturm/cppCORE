#ifndef LOGGINGWORKER_H
#define LOGGINGWORKER_H

#include "cppCORE_global.h"
#include <QRunnable>
#include <QString>

/// This class writes logging statements in a separate thread. It should help to avoid
/// the situation when the main thread hangs, if there is a problem with IO (due to
/// some blocking file operation)
class CPPCORESHARED_EXPORT LoggingWorker
    : public QRunnable
{
public:
    explicit LoggingWorker(QString file_name, QString message, QString level_str);
    void run() override;

private:
    QString file_name_;
    QString message_;
    QString level_str_;
};

#endif // LOGGINGWORKER_H
