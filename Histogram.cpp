#include <cmath>
#include <limits>
#include <algorithm>

#include "Exceptions.h"
#include "Histogram.h"
#include "Helper.h"
#include "Log.h"
#include "Settings.h"
#include <QStringList>
#include <QProcess>
#include <QStandardPaths>

Histogram::Histogram(double min, double max, double bin_size)
	: min_(min)
	, max_(max)
	, bin_size_(bin_size)
	, bin_sum_(0)
	, alpha_(std::numeric_limits<double>::quiet_NaN())
{
	if (bin_size_<=0)
	{
		THROW(StatisticsException,"Cannot initialize histogram with non-positive bin size!");
	}

	if (min_>=max_)
	{
		THROW(StatisticsException,"Cannot initialize histogram with empty range!");
	}

	bins_.resize(ceil((max_-min_)/bin_size_));
}

double Histogram::maxValue(bool as_percentage) const
{
	if (bins_.size()==0)
	{
		THROW(StatisticsException,"No bins present!");
	}

	double max = *(std::max_element(bins_.begin(), bins_.end()));
	if(as_percentage)
	{
		return 100.0 * max / (double)bin_sum_;
	}
	return max;
}

double Histogram::minValue(bool as_percentage) const
{
	if (bins_.size()==0)
	{
		THROW(StatisticsException,"No bins present!");
	}

	double min = *(std::min_element(bins_.begin(), bins_.end()));
	if(as_percentage)
	{
		return 100.0 * min / (double)bin_sum_;
	}
	return min;
}

double Histogram::binValue(int index, bool as_percentage) const
{
	if (index<0 || index>=(int)bins_.size())
	{
        THROW(StatisticsException,"Index " + QString::number(index) + " out of range (0-" + QString::number(bins_.size()-1) + ")!");
	}

	double value = bins_[index];
	if(as_percentage)
	{
		return 100.0 * value / (double)bin_sum_;
	}
	return value;
}

double Histogram::startOfBin(int index) const
{
	if (index<0 || index>=(int)bins_.size())
	{
        THROW(StatisticsException,"Index " + QString::number(index) + " out of range (0-" + QString::number(bins_.size()-1) + ")!");
	}

    return bin_size_*index + min_;
}

double Histogram::binValue(double val, bool as_percentage, bool ignore_bounds_errors) const
{
	double value = bins_[binIndex(val, ignore_bounds_errors)];
	if(as_percentage)
	{
		return 100.0 * value / (double)bin_sum_;
	}
	return value;
}

int Histogram::binIndex(double val, bool ignore_bounds_errors) const
{
	if (!ignore_bounds_errors && (val < min_ || val > max_))
	{
		THROW(StatisticsException, "Requested position '" + QString::number(val) + "' not in range (" + QString::number(min_) + "-" + QString::number(max_) + ")!");
	}

	int index = floor ( (val-min_) / (max_-min_) * bins_.size());

    return BasicStatistics::bound(SIZE_TO_INT(index), SIZE_TO_INT(0), SIZE_TO_INT(bins_.size()-1));
}


void Histogram::print(QTextStream& stream, QString indentation, int position_precision, int data_precision, bool ascending) const
{
    for (int i=0; i<bins_.count(); ++i)
	{
        int index = ascending ? i : bins_.count()-i-1;
        double start = startOfBin(index);
        double end = start + bin_size_;
        if (!ascending) std::swap(start, end);
        stream << indentation << QString::number(start, 'f', position_precision) << "-" << QString::number(end, 'f', position_precision) << ": " << QString::number(binValue(index), 'f', data_precision) << "\n";
	}
}

QVector<double> Histogram::yCoords(bool as_percentage)
{
	if (as_percentage)
	{
		QVector<double> tmp(bins_);
		for (int i=0; i<tmp.count(); ++i)
		{
			tmp[i] = 100.0 * tmp[i] / bin_sum_;
		}
		return tmp;
	}
	else
	{
		return bins_;
	}
}

void Histogram::store(QString filename, bool x_log_scale, bool y_log_scale, double min_offset)
{
	//create python script
	QStringList script;
	script.append("from numpy import nan");
	script.append("import matplotlib as mpl");
	script.append("mpl.use('Agg')");
	script.append("import matplotlib.pyplot as plt");
	script.append("plt.figure(figsize=(10, 4), dpi=100)");
	if(ylabel_!="") script.append("plt.ylabel('" + ylabel_ + "')");
	if(xlabel_!="") script.append("plt.xlabel('" + xlabel_ + "')");
	//scale axis
	double x_min = min();
	double y_min = minValue();
	if(x_log_scale)
	{
		script.append("plt.xscale(\"log\")");
		//add offset to avoid 0 in log-scale plot
		if(x_min == 0.0)  x_min += min_offset;
	}
	if(y_log_scale)
	{
		script.append("plt.yscale(\"log\")");
		//add offset to avoid 0 in log-scale plot
		if(y_min == 0.0) y_min += min_offset;
	}
	script.append("plt.tick_params(axis='x', which='both', bottom='off', top='off')");
	script.append("plt.tick_params(axis='y', which='both', left='off', right='off')");
	script.append("plt.ylim(" + QString::number(y_min) + "," + QString::number(maxValue()+0.2*maxValue()) + ")");
	script.append("plt.xlim(" + QString::number(x_min) + "," + QString::number(max()) + ")");



	//data
	QString yvaluestring = "";
	QString xvaluestring = QString::number(min_);
	QVector<double> x = xCoords();
	QVector<double> y = yCoords();
	for (int i=0; i<x.size(); ++i)
	{
		for(int j=0; j<y[i];++j)
		{
			yvaluestring += "," + QString::number(x[i]);
		}

		xvaluestring += "," + QString::number(x[i]+0.5*binSize());
	}
	xvaluestring = "[" + xvaluestring + "]";	//remove first ,
	yvaluestring = "[" + yvaluestring.remove(0,1) + "]";

	script.append("plt.hist(" + yvaluestring + ", bins=" + xvaluestring + ", rwidth = 0.8, edgecolor='none')");

	//file handling
	script.append("plt.savefig('" + filename.replace("\\", "/") + "', bbox_inches=\'tight\', dpi=100)");

	//check if python is installed
	QString python_exe = Settings::path("python_exe", true);
	if (python_exe=="") python_exe  = QStandardPaths::findExecutable("python3");
	if (python_exe!="")
	{
		QString scriptfile = Helper::tempFileName(".py");
		Helper::storeTextFile(scriptfile, script);

		//execute scipt
		QProcess process;
		process.setProcessChannelMode(QProcess::MergedChannels);
		process.start(python_exe, QStringList() << scriptfile);
		bool success = process.waitForFinished(-1);
		QByteArray output = process.readAll();
		if (!success || process.exitCode()>0)
		{
			THROW(ProgrammingException, "Could not execute python script:\n" + scriptfile + "\n Error message is: " + output);
		}

		//remove temporary file
		QFile::remove(scriptfile);
	}
	else
	{
		Log::warn("Python executable not found in PATH - skipping plot generation!");
	}
}

void Histogram::storeCombinedHistogram(QString filename, QList<Histogram> histograms, QString xlabel, QString ylabel)
{
	//check that all histograms have the same bins and labels
	double min = 0;
	double max = 0;
	double minValue = 0;
	double maxValue = 0;
	foreach(Histogram h, histograms)
	{
		if(min > h.min())	min = h.min();
		if(max < h.max())	max = h.max();
		if(minValue > h.minValue())	minValue = h.minValue();
		if(maxValue < h.maxValue())	maxValue = h.maxValue();
	}

	//create python script
	QStringList script;
	script.append("import matplotlib as mpl");
	script.append("mpl.use('Agg')");
	script.append("import matplotlib.pyplot as plt");
	script.append("plt.figure(figsize=(10, 4), dpi=100)");
	if(ylabel!="") script.append("plt.ylabel('" + ylabel + "')");
	if(xlabel!="") script.append("plt.xlabel('" + xlabel + "')");

	script.append("plt.tick_params(axis='x', which='both', bottom='off', top='off')");
	script.append("plt.tick_params(axis='y', which='both', left='off', right='off')");
	script.append("plt.ylim(" + QString::number(minValue) + "," + QString::number(maxValue + 0.1*maxValue) + ")");
	script.append("plt.xlim(" + QString::number(min) + "," + QString::number(max) + ")");

	//data
	foreach(Histogram h, histograms)
	{
		QString yvaluestring = "";
		QString xvaluestring = QString::number(h.min());
		QVector<double> x = h.xCoords();
		QVector<double> y = h.yCoords();
		for (int i=0; i<x.size(); ++i)
		{
			for(int j=0; j<y[i];++j)
			{
				yvaluestring += "," + QString::number(x[i]);
			}
			xvaluestring += "," + QString::number(x[i]+0.5*h.binSize());
		}
		xvaluestring = "[" + xvaluestring + "]";	//remove first ','
		yvaluestring = "[" + yvaluestring.remove(0,1) + "]";

        script.append("plt.hist(" + yvaluestring + ", bins=" + xvaluestring + ", rwidth = 0.8, " + (h.color_.size()>0 ? "color=" + h.color_ + ", " : "") + (BasicStatistics::isValidFloat(h.alpha_) ? "alpha=" + QString::number(h.alpha_) + "," : "") + " edgecolor='none', label = '" + h.label_ + "')");
	}
	script.append("plt.legend(loc='upper right',fontsize=10)");

	//file handling
	script.append("plt.savefig('" + filename.replace("\\", "/") + "', bbox_inches=\'tight\', dpi=100)");

	//check if python is installed
	QString python_exe = Settings::path("python_exe", true);
	if (python_exe=="") python_exe  = QStandardPaths::findExecutable("python3");
	if (python_exe!="")
	{
		QString scriptfile = Helper::tempFileName(".py");
		Helper::storeTextFile(scriptfile, script);

		//execute scipt
		QProcess process;
		process.setProcessChannelMode(QProcess::MergedChannels);
		process.start(python_exe, QStringList() << scriptfile);
		bool success = process.waitForFinished(-1);
		QByteArray output = process.readAll();
		if (!success || process.exitCode()>0)
		{
			THROW(ProgrammingException, "Could not execute python script:\n" + scriptfile + "\n Error message is: " + output);
		}

		//remove temporary file
		QFile::remove(scriptfile);
	}
	else
	{
		Log::warn("Python executable not found in PATH - skipping plot generation!");
	}
}

