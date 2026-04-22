#ifndef SCATTERPLOT_H
#define SCATTERPLOT_H

#include "cppCORE_global.h"
#include <QString>
#include <QList>
#include <QHash>

// Creates a scatter plot PNG image
class CPPCORESHARED_EXPORT ScatterPlot
{
public:
	ScatterPlot();
	void setValues(const QList< std::pair<double,double> >& values, const QList<QString>& colors = QList<QString>())
	{
		points_.clear();
		points_.append(values);
		colors_.clear();
		colors_.append(colors);
	}
	void convertChromToX();
	void setXLabel(QString xlabel)
	{
		xlabel_ = xlabel;
	}
	void setXRange(double xmin, double xmax)
	{
		xmin_ = xmin;
		xmax_ = xmax;
		xrange_set_ = true;
	}
	void setYLabel(QString ylabel)
	{
		ylabel_ = ylabel;
	}
	void setYRange(double ymin, double ymax)
	{
		ymin_ = ymin;
		ymax_ = ymax;
		yrange_set_ = true;
	}
	void setYLogScale(bool yscale_log)
	{
		yscale_log_ = yscale_log;
	}
	void addVLine(double x)
	{
		vlines_.append(x);
	}
	void addColorLegend(QString color,QString desc)
	{
		color_legend_.insert(color, desc);
	}
	void store(QString filename);

protected:
	//variables to store the plot data
	QList<std::pair<double,double>> points_;
	QList<double> vlines_;
	QList<QString> colors_;
	QHash<QString,QString> color_legend_;
	QString xlabel_;
	QString ylabel_;
	double ymin_;
	double ymax_;
	double xmin_;
	double xmax_;

	//variables to store the meta data
	bool yrange_set_;
	bool xrange_set_;
	bool yscale_log_;	
};

#endif // SCATTERPLOT_H
