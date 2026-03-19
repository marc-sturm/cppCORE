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
	QBarSeries* series = new QBarSeries();

	for (int i = 0; i < bars_.size(); ++i)
	{
		QBarSet* set = new QBarSet(labels_[i]);
		QString color = (colors_.size() > 0) ? colors_[i] : "blue";

		set->setColor(QColor::fromString(color));

		if (is_legend_visible_)
		{
			*set << bars_[i];
		}
		else
		{
			for (int j = 0; j < bars_.size(); ++j)
			{
				if (j == i) *set << bars_[i];
				else *set << 0.0;
			}
		}

		series->append(set);
	}

	chart->addSeries(series);

	// X axis categories
	QBarCategoryAxis* axis_x = new QBarCategoryAxis();
	QStringList categories;
	if (!labels_.isEmpty() && labels_.size() == bars_.size())
	{
		categories = labels_;
	}
	else
	{
		for (int i = 0; i < bars_.size(); ++i)
		{
			categories << QString::number(i);
		}
	}

	// legend
	chart->legend()->setVisible(is_legend_visible_);
	chart->legend()->setAlignment(Qt::AlignRight);

	if (is_legend_visible_)
	{
		axis_x->append(QStringList(""));
	}
	else
	{
		axis_x->append(categories);
		if (!xlabel_.isEmpty()) axis_x->setTitleText(xlabel_);
		axis_x->setLabelsAngle(-90);
	}

	chart->addAxis(axis_x, Qt::AlignBottom);
	series->attachAxis(axis_x);

	// Y axis
	QValueAxis* axis_y = new QValueAxis();
	if (!ylabel_.isEmpty()) axis_y->setTitleText(ylabel_);

	if (BasicStatistics::isValidFloat(ymin_) && BasicStatistics::isValidFloat(ymax_))
	{
		axis_y->setRange(ymin_, ymax_);
	}

	chart->addAxis(axis_y, Qt::AlignLeft);
	series->attachAxis(axis_y);

	axis_x->setGridLineVisible(false);
	axis_y->setGridLineVisible(false);

	plot_utils->saveAsPng(filename, 1000, 400);
}
