#include "VersatileTextStream.h"

VersatileTextStream::VersatileTextStream(QString file_name, bool stdin_if_empty)
	: file_name_(file_name)
	, file_(file_name_, stdin_if_empty)
{
	file_.open(QFile::ReadOnly|QFile::Text);

	//check it is not UTF16 or UTF32
	QByteArray first_line = file_.readLine();
	if (first_line.startsWith(QByteArray::fromHex("FFFE0000"))) THROW(FileParseException, "Unsupported encoding 'UTF32LE' used in " + file_name);
	if (first_line.startsWith(QByteArray::fromHex("0000FEFF"))) THROW(FileParseException, "Unsupported encoding 'UTF32BE' used in " + file_name);
	if (first_line.startsWith(QByteArray::fromHex("FFFE"))) THROW(FileParseException, "Unsupported encoding 'UTF16LE' used in " + file_name);
	if (first_line.startsWith(QByteArray::fromHex("FEFF"))) THROW(FileParseException, "Unsupported encoding 'UTF16BE' used in " + file_name);
	if (!file_.seek(0)) THROW(FileParseException, "Error while peeking into file " + file_name);
}
