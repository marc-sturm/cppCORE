#ifndef BARPLOT_H
#define BARPLOT_H

#include "cppCORE_global.h"
#include <QString>
#include <QList>

// Creates a bar plot PNG image
class CPPCORESHARED_EXPORT BarPlot
{
public:
	BarPlot();

	void setValues(const QList<int>& values, const QList<QString>& labels, const QList<QString>& colors = QList<QString>());
	void setValues(const QList<double>& values, const QList<QString>& labels, const QList<QString>& colors = QList<QString>());
	void setXLabel(const QString& x_label)
	{
		xlabel_ = x_label;
	}
	void setYLabel(const QString& y_label)
	{
		ylabel_ = y_label;
	}
	void setXRange(double min, double max)
	{
		xmin_ = min;
		xmax_ = max;
	}
	void setYRange(double min, double max)
	{
		ymin_ = min;
		ymax_ = max;
	}

	void store(QString filename);

protected:
	QList<double> bars_;
	QList<QString> labels_;
	QList<QString> colors_;	
	QString xlabel_;
	QString ylabel_;
	double ymin_;
	double ymax_;
	double xmin_;
	double xmax_;
};

#endif
