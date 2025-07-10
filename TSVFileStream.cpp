#include "TSVFileStream.h"
#include "Helper.h"
#include <QStringList>

TSVFileStream::TSVFileStream(QString filename, char separator, char comment)
	: filename_(filename)
	, separator_(separator)
	, comment_(comment)
	, double_comment_(2, comment)
	, file_(filename)
	, line_(0)
{
	//open
	bool open_status = true;
	if (filename.isEmpty())
	{
		open_status = file_.open(stdin, QFile::ReadOnly | QFile::Text);
	}
	else
	{
		open_status = file_.open(QFile::ReadOnly | QFile::Text);
	}
	if (!open_status)
	{
		THROW(FileAccessException, "Could not open file for reading: '" + filename + "'!");
	}

	//read comments and headers
	next_line_ = double_comment_;
	while(next_line_.startsWith(comment))
	{
		if (next_line_.startsWith(double_comment_))
		{
			if (next_line_.trimmed()!=double_comment_ && header_.isEmpty())
			{
				comments_ << next_line_;
			}
		}
		else if (next_line_.startsWith(comment))
		{
			header_ = next_line_.mid(1).split(separator);
		}

		next_line_ = file_.readLine();
		while (next_line_.endsWith('\n') || next_line_.endsWith('\r')) next_line_.chop(1);
		++line_;
	}

	//no first line
	if (file_.atEnd() && next_line_.isEmpty()) next_line_ = QByteArray();

	//determine number of columns if no header is present
	if (header_.isEmpty())
	{
		for(int i=0; i<next_line_.split(separator).count(); ++i)
		{
			header_.append("");
		}
	}
}

TSVFileStream::~TSVFileStream()
{
	file_.close();
}

QByteArrayList TSVFileStream::readLine()
{
	//handle first content line
	if (!next_line_.isNull())
	{
		if (next_line_.isEmpty())
		{
			next_line_ = QByteArray();
			return QByteArrayList();
		}
		QByteArrayList parts = next_line_.split(separator_);
		if (parts.count()!=columns()) THROW(FileParseException, "Expected " + QString::number(columns()) + " columns, but got " + QString::number(parts.count()) + " columns in line 1: " + next_line_);

		next_line_ = QByteArray();
		return parts;
	}

	//handle second to last content line
	QByteArray line = file_.readLine();
	while (line.endsWith('\n') || line.endsWith('\r')) line.chop(1);
	++line_;

	if (line.isEmpty()) return QByteArrayList();

	if (line.startsWith(double_comment_)) //comments between lines are ignored
	{
		return readLine();
	}

	QByteArrayList parts = line.split(separator_);
	if (parts.count()!=columns()) THROW(FileParseException, "Expected " + QString::number(columns()) + " columns, but got " + QString::number(parts.count()) + " columns in line " + QString::number(line_) + ": " + line);

	return parts;
}

int TSVFileStream::colIndex(QByteArray name, bool error_when_missing)
{
	//find matching indices
	QVector<int> hits;
	for (int i=0; i<columns(); ++i)
	{
		if (header_[i]==name)
		{
			hits.append(i);
		}
	}

    qint64 hits_count = hits.count();
	//error handling
    if (hits_count!=1)
	{
		if (error_when_missing)
		{
            if (hits_count==0)
			{
				THROW(CommandLineParsingException, "Could not find column name '" + name + "' in column headers!");
			}
            if (hits_count>1)
			{
				THROW(CommandLineParsingException, "Found column name '" + name + "' more than once in column headers!");
			}
		}
		else
		{
			return -1;
		}
	}

	return hits[0];
}

QVector<int> TSVFileStream::checkColumns(const QByteArrayList& col_names, bool numeric)
{
	QVector<int> col_indices;

	if (numeric)
	{
		foreach(const QByteArray& part, col_names)
		{
			int col = Helper::toInt(part, "column number");
			if (col<1 || col>columns())
			{
				THROW(CommandLineParsingException, "1-based column number '" + part + "' out of range (max is " + QString::number(columns()) + ")!");
			}
			col_indices.append(col-1);
		}
	}
	else
	{
		foreach(const QByteArray& part, col_names)
		{
			col_indices.append(colIndex(part, true));
		}
	}

	return col_indices;
}
