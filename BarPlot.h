#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include "cppCORE_global.h"
#include <QString>
#include <QVector>
#include <QHash>

class CPPCORESHARED_EXPORT BarPlot
{
public:
	BarPlot();

	void setValues(const QList<int>& values, const QList<QString>& labels, const QList<QString>& colors = QList<QString>());
	void setXLabel(const QString& x_label)
	{
		xlabel_ = x_label;
	}
	void setYLabel(const QString& y_label)
	{
		ylabel_ = y_label;
	}
	void setDPI(int dpi)
	{
		dpi_ = dpi;
	}
	void setXRange(int min, int max)
	{
		xmin_ = min;
		xmax_ = max;
	}
	void setYRange(int min, int max)
	{
		ymin_ = min;
		ymax_ = max;
	}
	void addColorLegend(QString color,QString desc);
	void store(QString filename);

protected:
	//variables to store the plot data
	QList<int> bars_;
	QList<QString> labels_;
	QList<QString> colors_;
	QHash<QString,QString> color_legend_;
	QString xlabel_;
	QString ylabel_;
	int ymin_;
	int ymax_;
	int xmin_;
	int xmax_;
	int dpi_;
};

#endif // HISTOGRAM_H
