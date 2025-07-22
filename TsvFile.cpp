#include "TsvFile.h"
#include "Exceptions.h"
#include "Helper.h"
#include "VersatileTextStream.h"

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

void TsvFile::setComments(const QStringList& comments)
{
	comments_.clear();
	foreach(const QString& comment, comments)
	{
		addComment(comment);
	}
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

const QStringList& TsvFile::operator[](int i) const
{
	if (i<0 || i>=rows_.count())
	{
		THROW(ProgrammingException, "TsvFile: table has " + QString::number(rows_.count()) + " rows, but row with index " + QString::number(i) + " was requested.");
	}

	return rows_[i];
}

int TsvFile::columnIndex(const QString& column, bool throw_if_not_found) const
{
	for (int c=0; c<headers_.count(); ++c)
	{
		if (headers_[c]==column) return c;
	}

	if (throw_if_not_found)	THROW(ProgrammingException, "Column '" + column + "' not found in TsvFile!");

	return -1;
}

QStringList TsvFile::extractColumn(int c)
{
	if (c<0 || c>=headers_.count())
	{
		THROW(ProgrammingException, "TsvFile: table has " + QString::number(headers_.count()) + " columns, but column with index " + QString::number(c) + " was requested.");
	}

	QStringList output;
	foreach(const QStringList& row, rows_)
	{
		output << row[c];
	}
	return output;
}

void TsvFile::removeColumn(int c)
{
	if (c<0 || c>=headers_.count())
	{
		THROW(ProgrammingException, "TsvFile: table has " + QString::number(headers_.count()) + " columns, but column with index " + QString::number(c) + " was requested.");
	}

	headers_.removeAt(c);
	for (int i=0; i<rows_.count(); ++i)
	{
		rows_[i].removeAt(c);
	}
}

void TsvFile::load(QString filename, bool use_string_hash)
{
	VersatileTextStream stream(filename);
	while (!stream.atEnd())
	{
		QString line = stream.readLine();

		//skip empty lines
		if (line.isEmpty()) continue;

		//header lines
		if (line[0]=='#')
		{
            if (line.size()>1 && line[1]=='#') //comment
			{
				addComment(line);
			}
			else //header
			{
				QStringList parts = line.mid(1).split('\t');
				foreach(QString part, parts)
				{
					addHeader(part);
				}
			}
			continue;
		}

		//content lines

		if (use_string_hash)
		{
			QStringList tmp;
			foreach(QString entry, line.split('\t'))
			{
				if (!hash_.contains(entry)) hash_.insert(entry, entry);
				tmp.append(hash_[entry]);
			}
			rows_ << tmp;
		}
		else
		{
			addRow(line.split('\t'));
		}
	}

	hash_.clear();
}

void TsvFile::store(QString filename) const
{
	auto file = Helper::openFileForWriting(filename);
	QTextStream stream(file.data());
	toStream(stream);
}

QString TsvFile::toString() const
{
	QString output;
	QTextStream stream(&output);
	stream.setCodec("UTF-8");
	toStream(stream);

	return output;
}

void TsvFile::toStream(QTextStream& stream) const
{
	//comment
	foreach(const QString& comment, comments_)
	{
		stream << comment << '\n';
	}

	//header
	stream << '#';
	for(int i=0; i<headers_.count(); ++i)
	{
		if (i!=0) stream << '\t';
		stream << headers_[i];
	}
	stream << '\n';

	//rows
	foreach(const QStringList& row, rows_)
	{
		for(int i=0; i<row.count(); ++i)
		{
			if (i!=0) stream << '\t';
			stream << row[i];
		}
		stream << '\n';
	}
}
