#include "VersatileTextStream.h"
#include "Helper.h"

VersatileTextStream::VersatileTextStream(QString file_name)
	: file_name_(file_name)
	, file_(file_name_)
{
	file_.open();
}
