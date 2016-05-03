#include "ScatterPlot.h"
#include "Exceptions.h"
#include "Helper.h"
#include "Log.h"
#include "BasicStatistics.h"
#include <QStringList>
#include <limits>
#include <QProcess>
#include <QStandardPaths>

ScatterPlot::ScatterPlot()
	: yrange_set_(false), xrange_set_(false)
{
}

void ScatterPlot::setValues(const QList< QPair<double,double> >& values)
{
	points_.clear();
	points_.append(values);
}

void ScatterPlot::setXLabel(QString xlabel)
{
	xlabel_ = xlabel;
}

void ScatterPlot::setYLabel(QString ylabel)
{
	ylabel_ = ylabel;
}

void ScatterPlot::setXRange(double xmin, double xmax)
{
	xmin_ = xmin;
	xmax_ = xmax;
	xrange_set_ = true;
}

void ScatterPlot::setYRange(double ymin, double ymax)
{
	ymin_ = ymin;
	ymax_ = ymax;
	yrange_set_ = true;
}

void ScatterPlot::store(QString filename)
{
	//check if python is installed
	QString python_exe = QStandardPaths::findExecutable("python");
	if (python_exe=="")
	{
		Log::warn("Python executable not found in PATH - skipping plot generation!");
		return;
	}

	//create python script
	QString scriptfile = Helper::tempFileName(".py");
	QStringList script;
	script.append("import matplotlib as mpl");
	script.append("mpl.use('Agg')");
	script.append("import matplotlib.pyplot as plt");
	script.append("plt.figure(figsize=(6, 4), dpi=100)");
	if(ylabel_!="") script.append("plt.ylabel('" + ylabel_ + "')");
	if(xlabel_!="") script.append("plt.xlabel('" + xlabel_ + "')");
	if(yscale_log_)	script.append("plt.set_yscale('log')");
	if(!yrange_set_)
	{
		double min = std::numeric_limits<double>::max();
		double max = -std::numeric_limits<double>::max();
		QPair<double,double> point;
		foreach(point, points_)
		{
				min = std::min(point.second, min);
				max = std::max(point.second, max);
		}
		ymin_ = min-0.01*(max-min);
		ymax_ = max+0.01*(max-min);;
	}
	if(!xrange_set_)
	{
		double min = std::numeric_limits<double>::max();
		double max = -std::numeric_limits<double>::max();
		QPair<double,double> point;
		foreach(point, points_)
		{
				min = std::min(point.first, min);
				max = std::max(point.first, max);
		}
		xmin_ = min-0.01*(max-min);
		xmax_ = max+0.01*(max-min);;
	}
	if(BasicStatistics::isValidFloat(ymin_) && BasicStatistics::isValidFloat(ymax_))
	{
		script.append("plt.ylim(" + QString::number(ymin_) + "," + QString::number(ymax_) + ")");
	}
	if(BasicStatistics::isValidFloat(xmin_) && BasicStatistics::isValidFloat(xmax_))
	{
		script.append("plt.xlim(" + QString::number(xmin_) + "," + QString::number(xmax_) + ")");
	}
	QString xvaluestring = "";
	QString yvaluestring = "";
	if (points_.count()>0)
	{
		xvaluestring += "[" + QString::number(points_[0].first);
		yvaluestring += "[" + QString::number(points_[0].second);
		for (int i=1; i<points_.count(); ++i)
		{
			xvaluestring += ","+QString::number(points_[i].first);
			yvaluestring += ","+QString::number(points_[i].second);
		}
		xvaluestring += "],";
		yvaluestring += "],";
	}
	script.append("plt.scatter(" + xvaluestring + yvaluestring + "3)");
	script.append("plt.savefig('" + filename.replace("\\", "/") + "', bbox_inches=\'tight\', dpi=100)");

	Helper::storeTextFile(scriptfile, script);

	//execute scipt
	QProcess process;
	process.setProcessChannelMode(QProcess::MergedChannels);
	process.start(python_exe, QStringList() << scriptfile);
	if (!process.waitForFinished(-1) || process.readAll().contains("rror"))
	{
		THROW(ProgrammingException, "Could not execute python script! Error message is: " + process.errorString());
	}

	//remove temporary file
	QFile::remove(scriptfile);
}
