#ifndef TSVFILE_H
#define TSVFILE_H

#include "cppCORE_global.h"
#include <QStringList>
#include <QTextStream>

///TSV file representation (row-wise)
class CPPCORESHARED_EXPORT TsvFile
{
public:
	TsvFile();

	void addComment(const QString& comment);
	const QStringList& comments() const { return comments_; }
	void setComments(const QStringList& comments);

	void addHeader(const QString& header);
	const QStringList& headers() const;

	void addRow(const QStringList& row);
	const QStringList& operator[](int i) const;
	int count() const { return rows_.count(); }

	//Returns the column index. Throws an exception if the column does not exist, or returns -1.
	int columnIndex(const QString& column, bool throw_if_not_found=true) const;

	//Loads a TSV file with '#' as start of header line and '##' as start of comment lines.
	void load(QString filename);
	//Stores the TSV file to a file.
	void store(QString filename) const;
	//Converts the TSV file to string.
	QString toString() const;

	//Creates a columns representation (slow).
	QStringList extractColumn(int c);
	//Removes a column.
	void removeColumn(int c);

private:
	QStringList comments_;
	QStringList headers_;
	QList<QStringList> rows_;

	//Stream writer helper
	void toStream(QTextStream& steam) const;
};

#endif // TSVFILE_H
