#include "BarPlot.h"
#include "Exceptions.h"
#include "Helper.h"
#include "Log.h"
#include "BasicStatistics.h"
#include <QStringList>
#include <limits>
#include <QProcess>
#include <QStandardPaths>

BarPlot::BarPlot()
{
}

void BarPlot::setValues(const QList<int>& values, const QList<QString>& labels, const QList<QString>& colors)
{
	for(int i=0;i<values.count();++i)
	{
		bars_.append(double(values[i]));
		labels_.append(labels[i]);
		colors_.append(colors);
	}
}


void BarPlot::setValues(const QList<double>& values, const QList<QString>& labels, const QList<QString>& colors)
{
	for(int i=0;i<values.count();++i)
	{
		bars_.append(values[i]);
		labels_.append(labels[i]);
		colors_.append(colors);
	}
}


void BarPlot::addColorLegend(QString color, QString desc)
{
	color_legend_.insert(color, desc);
}

void BarPlot::store(QString filename)
{
	//create python script
	QStringList script;
	script.append("import matplotlib as mpl");
	script.append("mpl.use('Agg')");
	script.append("import matplotlib.pyplot as plt");
	script.append("import matplotlib.patches as mpatches");
	script.append("plt.figure(figsize=(10, 4), dpi=100)");
	if(ylabel_!="") script.append("plt.ylabel('" + ylabel_ + "')");
	if(xlabel_!="") script.append("plt.xlabel('" + xlabel_ + "')");

	if(BasicStatistics::isValidFloat(xmin_) && BasicStatistics::isValidFloat(xmax_))
	{
		script.append("plt.xlim(" + QString::number(xmin_) + "," + QString::number(xmax_) + ")");
	}
	if(BasicStatistics::isValidFloat(ymin_) && BasicStatistics::isValidFloat(ymax_))
	{
		script.append("plt.ylim(" + QString::number(ymin_) + "," + QString::number(ymax_) + ")");
	}

	//data
	QString xvaluestring = "";
	QString yvaluestring = "";
	xvaluestring += "[" + QString::number(0);
	yvaluestring += "[" + QString::number(bars_[0]);
	for(int i=1;i<bars_.count();++i)
	{
		xvaluestring += ","+QString::number(i);
		yvaluestring += ","+QString::number(bars_[i]);
	}
	xvaluestring += "]";
	yvaluestring += "]";

	//labels
	QString labelstring = "";
	if(!labels_.empty())
	{
		labelstring += "['" + labels_[0]+"'";
		for(int i=1;i<bars_.count();++i)
		{
			labelstring += ",'" + labels_[i]+"'";
		}
		labelstring += "]";
	}

	//colors
	QString colorstring = "";
	if(!colors_.empty())
	{
		colorstring += "['" + colors_.at(0)+"'";
		for(int i=1;i<bars_.count();++i)
		{
			colorstring += ",'" + colors_.at(i)+"'";
		}
		colorstring += "]";
	}

	script.append("barlist=plt.bar(" + xvaluestring + "," + yvaluestring + ",align='center'" + (colorstring.isEmpty() ?  "" : ",color=" + colorstring ) + ")");
	script.append("plt.xticks(" + xvaluestring + (labelstring.isEmpty() ? "" : "," + labelstring) + ", size='xx-small',rotation=90,horizontalalignment='center')");
	script.append("plt.yticks(size=10)");
	script.append("plt.tick_params(axis='x',which='both',length=0)");

	//legend
	QString c = "";
	QString d = "";
	foreach(const QString& desc, color_legend_)
	{
		QString col = color_legend_.key(desc);
		c += "mpatches.Patch(color='" + col + "'),";
		d += "'" + desc + "',";
	}
	script.append("plt.legend((" + c + "),(" + d + "),fontsize=10)");

	//file handling
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
		if (!process.waitForFinished(-1) || process.readAll().contains("rror"))
		{
//			qDebug() << script.join("\n");
			THROW(ProgrammingException, "Could not execute python script! Error message is: " + process.errorString());
		}

		//remove temporary file
		QFile::remove(scriptfile);
	}
	else
	{
//		qDebug() << script.join("\n");
		Log::warn("Python executable not found in PATH - skipping plot generation!");
	}
}
