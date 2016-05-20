#ifndef SCATTERPLOT_H
#define SCATTERPLOT_H

#include "cppCORE_global.h"
#include <QString>
#include <QList>
#include <QPair>

///Creates a line plot PNG image (needs Python in the path - and matplotlib extension).
class CPPCORESHARED_EXPORT ScatterPlot
{
public:
	ScatterPlot();
	void setValues(const QList< QPair<double,double> >& values);
	void convertChromToX();
	void setXLabel(QString xlabel);
	void setXRange(double xmin, double xmax);
	void setYLabel(QString ylabel);
	void setYRange(double ymin, double ymax);
	void noXTicks()
	{
		noxticks_ = true;
	}
	void setYLogScale(bool yscale_log)
	{
		yscale_log_ = yscale_log;
	}
	void addVLine(double x);
	void store(QString filename);

protected:
	//variables to store the plot data
	QList< QPair<double,double> > points_;
	QList< double > vlines_;
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
	bool noxticks_;
};

#endif // SCATTERPLOT_H
