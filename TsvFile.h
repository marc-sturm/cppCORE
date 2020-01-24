#ifndef TSVFILE_H
#define TSVFILE_H

#include "cppCORE_global.h"
#include <QStringList>

///TSV file representation (row-wise)
class CPPCORESHARED_EXPORT TsvFile
{
public:
	TsvFile();

	//comments
	void addComment(const QString& comment);
	const QStringList& comments() const;

	//header
	void addHeader(const QString& header);
	const QStringList& headers() const;

	//rows
	void addRow(const QStringList& row);
	const QStringList& row(int i) const;
	int rowCount() const;

	//misc
	QString toString() const;

private:
	QStringList comments_;
	QStringList headers_;
	QList<QStringList> rows_;

};

#endif // TSVFILE_H
