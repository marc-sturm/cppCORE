#include "BarPlot.h"

#include <limits>
#include <QFontDatabase>
#include <QChartView>
#include <QBarCategoryAxis>
#include <QValueAxis>
#include <QLegend>
#include <QLineSeries>
#include <QAreaSeries>
#include <QStringList>
#include <QGraphicsLayout>

#include "Exceptions.h"
#include "Helper.h"
#include "Log.h"
#include "BasicStatistics.h"
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

void BarPlot::store(QString filename)
{
	if (bars_.isEmpty())
	{
		Log::warn("BarPlot does not have any bars to plot");
		return;
	}

	int width = 1000;
	int height = 400;

	PlotUtils* plot_utils = new PlotUtils();
	QChart* chart = plot_utils->getChart();

	QValueAxis* axis_x = new QValueAxis();
	// axis_x->setRange(0, bars_.size());
	axis_x->setRange(-0.5, bars_.size() - 0.5);
	axis_x->setTickCount(bars_.size()+1);
	axis_x->setLabelFormat("%d");
	axis_x->setLabelsVisible(false);

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

	plot_utils->applyFontSettings();
	QFont label_font = plot_utils->getLabelFont();

	// create bars using QAreaSeries
	for (int i = 0; i < bars_.size(); ++i)
	{
		QGraphicsTextItem* label = new QGraphicsTextItem(categories[i]);
		label->setFont(label_font);

		double value = bars_[i];

		QLineSeries* upper = new QLineSeries();
		QLineSeries* lower = new QLineSeries();

		// shift bars so they align with categories
		double left  = i - 0.5;
		double right = i + 0.5;		

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

		// need to set the chart size to get the real coordinates for labels
		chart->resize(width, height);
		chart->layout()->activate();

		QPointF valuePoint(i+0.5, 0);
		QPointF pixelPoint = chart->mapToPosition(valuePoint, area);

		QRectF rect = label->boundingRect();
		// placing a category label below the x axis
		label->setPos(pixelPoint.x() - (rect.width() / 2), chart->plotArea().bottom()+rect.height()+10);

		label->setRotation(-90);
		label->setParentItem(chart);
	}

	// grid lines
	axis_x->setGridLineVisible(false);
	axis_y->setGridLineVisible(true);

	plot_utils->overpaintAxisX(axis_x, axis_y, bars_.size() - 0.5);
	plot_utils->saveAsPng(filename, width, height);
}
