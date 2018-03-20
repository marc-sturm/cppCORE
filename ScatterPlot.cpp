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
	: yrange_set_(false), xrange_set_(false), yscale_log_(false), noxticks_(false)
{
}

void ScatterPlot::setValues(const QList< QPair<double,double> >& values, const QList< QString >& colors)
{
	points_.clear();
	points_.append(values);
	colors_.clear();
	colors_.append(colors);
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

void ScatterPlot::addColorLegend(QString color, QString desc)
{
	color_legend_.insert(color, desc);
}

void ScatterPlot::addVLine(double x)
{
	vlines_.append(x);
}

void ScatterPlot::store(QString filename)
{
	//create python script
	QStringList script;
	script.append("import matplotlib as mpl");
	script.append("mpl.use('Agg')");
	script.append("import matplotlib.pyplot as plt");
	script.append("import matplotlib.patches as mpatches");
	script.append("plt.figure(figsize=(6, 4), dpi=100)");
	if(ylabel_!="") script.append("plt.ylabel('" + ylabel_ + "')");
	if(xlabel_!="") script.append("plt.xlabel('" + xlabel_ + "')");
	if(yscale_log_)	script.append("plt.yscale('log')");
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
	script.append("plt.tick_params(axis='x', which='both', bottom='off', top='off')");
	script.append("plt.tick_params(axis='y', which='both', left='off', right='off')");
	if(noxticks_)	script.append("plt.tick_params(axis='x', which='both', bottom='off', top='off', labelbottom='off')");
	QString xvaluestring = "[";
	QString yvaluestring = "[";
	QString cvaluestring = "[";
	if (points_.count()>0)
	{
		xvaluestring += QString::number(points_[0].first);
		yvaluestring += QString::number(points_[0].second);
		if(colors_.count() > 0)	cvaluestring += "'"+colors_[0]+"'";
		else	cvaluestring += "'k'";
		for (int i=1; i<points_.count(); ++i)
		{
			xvaluestring += ","+QString::number(points_[i].first);
			yvaluestring += ","+QString::number(points_[i].second);
			if(colors_.count() > 0)	cvaluestring += ",'" + colors_[i] + "'";
			else	cvaluestring += ", 'k'";
		}
	}
	xvaluestring += "]";
	yvaluestring += "]";
	cvaluestring += "]";
	script.append("plt.scatter(" + xvaluestring + "," + yvaluestring + ",3,"+ cvaluestring + ",edgecolors='face')");
	foreach(double x, vlines_)
	{
		script.append("plt.plot(("+QString::number(x)+","+QString::number(x)+"),("+QString::number(ymin_)+","+QString::number(ymax_)+"),'k--')");
	}

	//legend
	if(color_legend_.count()>0)
	{
		QString c = "";
		QString d = "";
		foreach(const QString& desc, color_legend_)
		{
			QString col = color_legend_.key(desc);
			c += "mpatches.Patch(color='" + col + "'),";
			d += "'" + desc + "',";
		}
		script.append("plt.legend((" + c + "),(" + d + "),fontsize=10,bbox_to_anchor=(1.025,1), loc=2, borderaxespad=0.,frameon=0)");
	}

	script.append("plt.savefig('" + filename.replace("\\", "/") + "', bbox_inches=\'tight\', dpi=100)");

	//check if python is installed
	QString python_exe = QStandardPaths::findExecutable("python");
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
		if (!success || output.contains("rror"))
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
