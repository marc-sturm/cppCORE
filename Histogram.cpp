#include "Histogram.h"

#include <cmath>
#include <limits>
#include <algorithm>
#include <QChartView>
#include <QValueAxis>
#include <QLogValueAxis>
#include <QLegend>
#include <QLineSeries>
#include <QAreaSeries>

#include "Exceptions.h"
#include "PlotUtils.h"
#include "Log.h"
#include "Helper.h"

Histogram::Histogram(double min, double max, double bin_size)
	: min_(min)
	, max_(max)
	, bin_size_(bin_size)
	, bin_sum_(0)
	, alpha_(std::numeric_limits<double>::quiet_NaN())
{
	if (bin_size_<=0)
	{
		THROW(StatisticsException, "Cannot initialize histogram with non-positive bin size!");
	}

	if (min_>=max_)
	{
		THROW(StatisticsException, "Cannot initialize histogram with empty range!");
	}

	bins_.resize(ceil((max_-min_)/bin_size_));
}


double Histogram::maxValue(bool as_percentage) const
{
	if (bins_.size()==0)
	{
		THROW(StatisticsException,"No bins present!");
	}

	double max = *(std::max_element(bins_.begin(), bins_.end()));
	if(as_percentage)
	{
		return 100.0 * max / (double)bin_sum_;
	}
	return max;
}

double Histogram::minValue(bool as_percentage) const
{
	if (bins_.size()==0)
	{
		THROW(StatisticsException,"No bins present!");
	}

	double min = *(std::min_element(bins_.begin(), bins_.end()));
	if(as_percentage)
	{
		return 100.0 * min / (double)bin_sum_;
	}
	return min;
}

double Histogram::binValue(int index, bool as_percentage) const
{
	if (index<0 || index>=(int)bins_.size())
	{
		THROW(StatisticsException, "Index " + QString::number(index) + " out of range (0-" + QString::number(bins_.size()-1) + ")!");
	}

	double value = bins_[index];
	if(as_percentage)
	{
		return 100.0 * value / (double)bin_sum_;
	}
	return value;
}

double Histogram::startOfBin(int index) const
{
	if (index<0 || index>=(int)bins_.size())
	{
		THROW(StatisticsException, "Index " + QString::number(index) + " out of range (0-" + QString::number(bins_.size()-1) + ")!");
	}

	return bin_size_*index + min_;
}

double Histogram::binValue(double val, bool as_percentage, bool ignore_bounds_errors) const
{
	double value = bins_[binIndex(val, ignore_bounds_errors)];
	if(as_percentage)
	{
		return 100.0 * value / (double)bin_sum_;
	}
	return value;
}

int Histogram::binIndex(double val, bool ignore_bounds_errors) const
{
	if (!ignore_bounds_errors && (val < min_ || val > max_))
	{
		THROW(StatisticsException, "Requested position '" + QString::number(val) + "' not in range (" + QString::number(min_) + "-" + QString::number(max_) + ")!");
	}

	int index = floor ( (val-min_) / (max_-min_) * bins_.size());

	return BasicStatistics::bound(static_cast<qsizetype>(index), static_cast<qsizetype>(0), static_cast<qsizetype>(bins_.size()-1));
}


void Histogram::print(QTextStream& stream, QString indentation, int position_precision, int data_precision, bool ascending) const
{
	for (int i=0; i<bins_.count(); ++i)
	{
		int index = ascending ? i : bins_.count()-i-1;
		double start = startOfBin(index);
		double end = start + bin_size_;
		if (!ascending) std::swap(start, end);
		stream << indentation << QString::number(start, 'f', position_precision) << "-" << QString::number(end, 'f', position_precision) << ": " << QString::number(binValue(index), 'f', data_precision) << "\n";
	}
}

QVector<double> Histogram::yCoords(bool as_percentage)
{
	if (as_percentage)
	{
		QVector<double> tmp(bins_);
		for (int i=0; i<tmp.count(); ++i)
		{
			tmp[i] = 100.0 * tmp[i] / bin_sum_;
		}
		return tmp;
	}
	else
	{
		return bins_;
	}
}

void Histogram::store(QString filename, bool x_log_scale, bool y_log_scale, double min_offset)
{
	QVector<double> x = xCoords();
	QVector<double> y = yCoords();

	if (x.isEmpty() && y.isEmpty())
	{
		Log::warn("No data to plot histogram");
		return;
	}

	PlotUtils* plot_utils = new PlotUtils();
	QChart* chart = plot_utils->getChart();
	chart->legend()->hide(); // hide legend, since we are using one color

	// Fixing zero values for logarithmic scaling (for X and Y separately)
	for(int i = 0; i < x.size(); ++i)
	{
		if(x_log_scale && x[i] <= 0.0) x[i] = min_offset;
		double shifted = x[i] - (binSize() / 2);
		if (shifted <= 0.0) x[i] = min_offset + (binSize() / 2);
	}
	for(int i = 0; i < y.size(); ++i)
	{
		if(y_log_scale && y[i] <= 0.0) y[i] = min_offset;
	}

	double x_min = min();
	double y_min = minValue();

	// X axis
	QAbstractAxis *axis_x;
	if(x_log_scale)
	{
		QLogValueAxis *log_axis = new QLogValueAxis();
		log_axis->setBase(10);
		log_axis->setMinorTickCount(10);

		if(x_min == 0.0) x_min += min_offset;

		log_axis->setRange(x_min, max());
		axis_x = log_axis;
	}
	else
	{
		QValueAxis *value_axis = new QValueAxis();
		value_axis->setTickCount(10);
		value_axis->applyNiceNumbers();
		value_axis->setRange(x_min, max());
		axis_x = value_axis;
	}
	if (!xlabel_.isEmpty()) axis_x->setTitleText(xlabel_);
	chart->addAxis(axis_x, Qt::AlignBottom);

	// Y axis
	QAbstractAxis *axis_y;
	if(y_log_scale)
	{
		QLogValueAxis *log_axis = new QLogValueAxis();
		log_axis->setBase(10);
		log_axis->setMinorTickCount(10);

		if(y_min == 0.0) y_min += min_offset;

		log_axis->setRange(y_min, maxValue() + 0.2 * maxValue());
		axis_y = log_axis;

	}
	else
	{
		QValueAxis *value_axis = new QValueAxis();
		value_axis->setRange(y_min, maxValue() + 0.2 * maxValue());
		value_axis->applyNiceNumbers();
		axis_y = value_axis;
	}
	if (!ylabel_.isEmpty()) axis_y->setTitleText(ylabel_);
	chart->addAxis(axis_y, Qt::AlignLeft);

	// Render the bars
	QLineSeries *upper = new QLineSeries();
	QLineSeries *lower = new QLineSeries();

	double baseline = y_log_scale ? min_offset : 0.0;

	// centering each bin
	lower->append(x.first()-(binSize()/2), baseline);
	upper->append(x.first()-(binSize()/2), baseline);

	for (int i = 0; i < y.size(); ++i)
	{
		upper->append(x[i]-(binSize()/2), y[i]);
		double next_item = x[i]-(binSize()/2)+binSize();
		upper->append(next_item, y[i]);
		lower->append(next_item, baseline);
	}

	QAreaSeries *area = new QAreaSeries(upper, lower);
	area->setName(label_);

	QColor bar_color(Qt::blue); // set the default color
	bar_color.setAlphaF(0.8);
	area->setColor(bar_color);
	area->setBorderColor(bar_color.darker());

	chart->addSeries(area);
	area->attachAxis(axis_x);
	area->attachAxis(axis_y);


	// Fixing the color of x axis
	QLineSeries *upper_x = new QLineSeries();
	QLineSeries *lower_x = new QLineSeries();
	double x_start = x_log_scale ? min_offset : 0.0;
	double y_base  = y_log_scale ? min_offset : 0.0;

	lower_x->append(x_start, y_base);
	upper_x->append(max(), y_base);

	lower_x->append(max(), y_base);
	upper_x->append(max(), y_base);

	QAreaSeries *area_x = new QAreaSeries(upper_x, lower_x);
	area_x->setName("x_axis");

	QColor x_color(axis_x->gridLineColor());
	area_x->setColor(x_color);
	area_x->setBorderColor(x_color);
	chart->addSeries(area_x);
	area_x->attachAxis(axis_x);
	area_x->attachAxis(axis_y);

	plot_utils->applyFontSettings();
	plot_utils->saveAsPng(filename, 1000, 400);
}

void Histogram::storeCombinedHistogram(QString filename, QList<Histogram> histograms, QString xlabel, QString ylabel)
{
	if (histograms.isEmpty())
	{
		Log::warn("No histograms to build a combined histogram");
		return;
	}

	//check that all histograms have the same bins and labels
	double min = 0;
	double max = 0;
	double min_value = 0;
	double max_value = 0;
	foreach(Histogram h, histograms)
	{
		if(min > h.min()) min = h.min();
		if(max < h.max()) max = h.max();
		if(min_value > h.minValue()) min_value = h.minValue();
		if(max_value < h.maxValue()) max_value = h.maxValue();
	}

	PlotUtils* plot_utils = new PlotUtils();
	QChart* chart = plot_utils->getChart();
	chart->legend()->setVisible(true);
	chart->legend()->setAlignment(Qt::AlignTop);

	int n = histograms.size();
	for (int i = 0; i < n; ++i)
	{
		if (histograms[i].color_.isEmpty())
		{
			int hue = (i * 360 / n);  // evenly spaced hues
			QColor color = QColor::fromHsv(hue, 200, 230);
			histograms[i].setColor(color.name());
		}
	}

	QValueAxis *axis_x = new QValueAxis();
	axis_x->setRange(min, max);
	axis_x->setTickCount(10);
	axis_x->applyNiceNumbers();
	if (!xlabel.isEmpty()) axis_x->setTitleText(xlabel);

	QValueAxis *axis_y = new QValueAxis();
	axis_y->setRange(min_value, max_value);
	axis_y->setTickCount(10);
	axis_y->applyNiceNumbers();
	if (!ylabel.isEmpty()) axis_y->setTitleText(ylabel);

	chart->addAxis(axis_x, Qt::AlignBottom);
	chart->addAxis(axis_y, Qt::AlignLeft);	

	foreach (Histogram h, histograms)
	{
		QLineSeries *upper = new QLineSeries();
		QLineSeries *lower = new QLineSeries();

		QVector<double> x = h.xCoords();
		QVector<double> y = h.yCoords();

		// centering each bin
		lower->append(x.first()-(h.binSize()/2), 0);
		upper->append(x.first()-(h.binSize()/2), 0);

		for (int i = 0; i < y.size(); ++i)
		{
			upper->append(x[i]-(h.binSize()/2), y[i]);
			double next_item = x[i]-(h.binSize()/2)+h.binSize();
			upper->append(next_item, y[i]);
			lower->append(next_item, 0);
		}

		QAreaSeries *area = new QAreaSeries(upper, lower);
		area->setName(h.label_);

		QColor bar_color = h.color_;
		bar_color.setAlphaF(0.8);
		area->setColor(bar_color);
		area->setBorderColor(bar_color.darker());

		chart->addSeries(area);
		area->attachAxis(axis_x);
		area->attachAxis(axis_y);
	}

	plot_utils->applyFontSettings();
	plot_utils->overpaintAxisX(axis_x, axis_y, max);
	plot_utils->saveAsPng(filename, 1000, 400);
}
