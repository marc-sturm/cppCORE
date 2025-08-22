#include "VersatileTextStream.h"
#include "Helper.h"

VersatileTextStream::VersatileTextStream(QString file_name, bool stdin_if_empty)
	: file_name_(file_name)
	, file_(file_name_, stdin_if_empty)
{
	file_.open(QFile::ReadOnly|QFile::Text);
}
