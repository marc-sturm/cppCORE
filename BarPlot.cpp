#include "BarPlot.h"

#include <limits>
#include <QStringList>
#include <QApplication>

#include "Exceptions.h"
#include "Helper.h"
#include "Log.h"
#include "BasicStatistics.h"
#include "Settings.h"
#include "PlotUtils.h"


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

void BarPlot::setXLabel(const QString &x_label)
{
	xlabel_ = x_label;
}

void BarPlot::setYLabel(const QString &y_label)
{
	ylabel_ = y_label;
}

void BarPlot::setXRange(double min, double max)
{
	xmin_ = min;
	xmax_ = max;
}

void BarPlot::setYRange(double min, double max)
{
	ymin_ = min;
	ymax_ = max;
}

void BarPlot::setLegendVisible(const bool &visible)
{
	is_legend_visible_ = visible;
}

void BarPlot::store(QString filename)
{
	if (bars_.isEmpty())
	{
		Log::warn("BarPlot does not have any bars to plot");
		return;
	}

	PlotUtils* plot_utils = new PlotUtils();
	QChart* chart = plot_utils->createEmptyChart();

	QBarCategoryAxis* axis_x = new QBarCategoryAxis();
	QValueAxis* axis_y = new QValueAxis();

	QStringList categories;
	if (!labels_.isEmpty() && labels_.size() == bars_.size())
	{
		categories = labels_;
	}
	else
	{
		for (int i = 0; i < bars_.size(); ++i) categories << QString::number(i);
	}

	axis_x->append(categories);
	axis_x->setLabelsAngle(-90);  // rotated labels

	if (!xlabel_.isEmpty()) axis_x->setTitleText(xlabel_);

	if (!ylabel_.isEmpty()) axis_y->setTitleText(ylabel_);

	// y range
	if (BasicStatistics::isValidFloat(ymin_) && BasicStatistics::isValidFloat(ymax_))
	{
		axis_y->setRange(ymin_, ymax_);
	}
	else
	{
		double ymax = *std::max_element(bars_.begin(), bars_.end());
		axis_y->setRange(0, ymax * 1.1);
	}

	chart->addAxis(axis_x, Qt::AlignBottom);
	chart->addAxis(axis_y, Qt::AlignLeft);
	chart->legend()->hide();

	// create bars using QAreaSeries
	for (int i = 0; i < bars_.size(); ++i)
	{
		double value = bars_[i];

		QLineSeries* upper = new QLineSeries();
		QLineSeries* lower = new QLineSeries();

		// shift bars so they align with categories
		double left  = i - 0.4;
		double right = i + 0.4;

		upper->append(left, 0);
		upper->append(left, value);
		upper->append(right, value);
		upper->append(right, 0);

		lower->append(left, 0);
		lower->append(right, 0);

		QAreaSeries* area = new QAreaSeries(upper, lower);

		// legend label
		if (labels_.size() == bars_.size()) area->setName(labels_[i]);

		// color
		QString color_str = (colors_.size() > i) ? colors_[i] : "blue";
		QColor color(color_str);

		area->setColor(color);
		area->setBorderColor(color.darker());

		chart->addSeries(area);
		area->attachAxis(axis_x);
		area->attachAxis(axis_y);
	}

	// grid lines
	axis_x->setGridLineVisible(true);
	axis_y->setGridLineVisible(true);

	plot_utils->saveAsPng(filename, 1000, 400);
}
