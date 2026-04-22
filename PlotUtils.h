#ifndef PLOTUTILS_H
#define PLOTUTILS_H

#include "cppCORE_global.h"
#include <QString>
#include <QHash>
#include <QtCharts/QChartView>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>
#include <QtCharts/QLegend>

// Class that helps to render and export plots
class CPPCORESHARED_EXPORT PlotUtils
{
public:
	PlotUtils();
	QChart* getChart();
	void applyFontSettings();
	QFont getLabelFont();
	void overpaintAxisX(QValueAxis* axis_x, QValueAxis* axis_y, double max);
	void saveAsPng(QString filename, int width, int height);

private:
	QChart* chart_;
};

#endif // PLOTUTILS_H
