#include "TsvFile.h"
#include "Exceptions.h"

TsvFile::TsvFile()
{
}

void TsvFile::addComment(const QString& comment)
{
	if (!comment.startsWith("##"))
	{
		THROW(ProgrammingException, "TsvFile: comment must start with '##', but does not: " + comment);
	}
	if (comment.contains("\n"))
	{
		THROW(ProgrammingException, "TsvFile: comment must not contain newline, but does: " + comment);
	}

	comments_ << comment;
}

const QStringList& TsvFile::comments() const
{
	return comments_;
}

void TsvFile::addHeader(const QString& header)
{
	if (header.isEmpty())
	{
		THROW(ProgrammingException, "TsvFile: header entry must not be empty, but is!");
	}
	if (headers_.isEmpty() && header.startsWith('#'))
	{
		THROW(ProgrammingException, "TsvFile: first header entry must not start with '#', but does: " + header);
	}
	if (header.contains("\t") || header.contains("\n"))
	{
		THROW(ProgrammingException, "TsvFile: header must not contain newline or tab, but does: " + header);
	}

	if (!rows_.isEmpty())
	{
		THROW(ProgrammingException, "TsvFile: cannot add header after row data was already added!");
	}

	headers_ << header;
}

const QStringList& TsvFile::headers() const
{
	return headers_;
}

void TsvFile::addRow(const QStringList& row)
{
	if (row.count()!=headers_.count())
	{
		THROW(ProgrammingException, "TsvFile: " + QString::number(headers_.count()) + " columns expected, but added row as " + QString::number(row.count()) + " columns:\n" + row.join("\t"));
	}
	foreach(const QString& entry, row)
	{
		if (entry.contains("\t") || entry.contains("\n"))
		{
			THROW(ProgrammingException, "TsvFile: row entry must not contain newline or tab, but does: " + entry);
		}
	}

	rows_ << row;
}

const QStringList& TsvFile::row(int i) const
{
	if (i<0 || i>=rows_.count())
	{
		THROW(ProgrammingException, "TsvFile: table has " + QString::number(rows_.count()) + " rows, but row with index " + QString::number(i) + " was requested.");
	}

	return rows_[i];
}

int TsvFile::rowCount() const
{
	return rows_.count();
}

QString TsvFile::toString() const
{
	QStringList output;

	//comment
	foreach(QString comment, comments_)
	{
		output << comment.trimmed();
	}

	//header
	output << "#" + headers_.join("\t");

	//rows
	foreach(QStringList row, rows_)
	{
		output << row.join("\t");
	}

	return output.join("\n");
}
