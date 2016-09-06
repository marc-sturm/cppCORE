#include <cmath>
#include <limits>
#include <algorithm>

#include "Exceptions.h"
#include "Histogram.h"
#include "BasicStatistics.h"

Histogram::Histogram(double min, double max, double bin_size)
	: min_(min)
	, max_(max)
	, bin_size_(bin_size)
	, bin_sum_(0)
{
	if (bin_size_<=0)
	{
		THROW(StatisticsException,"Cannot initialize histogram with non-positive bin size!");
	}

	if (min_>=max_)
	{
		THROW(StatisticsException,"Cannot initialize histogram with empty range!");
	}

	bins_.resize(ceil((max_-min_)/bin_size_));
}

double Histogram::min() const
{
	return min_;
}

double Histogram::max() const
{
	return max_;
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

double Histogram::binSize() const
{
	return bin_size_;
}

int Histogram::binCount() const
{
	return bins_.size();
}

int Histogram::binSum()
{
	return bin_sum_;
}

double Histogram::binValue(int index, bool as_percentage) const
{
	if (index<0 || index>=(int)bins_.size())
	{
        THROW(StatisticsException,"Index " + QString::number(index) + " out of range (0-" + QString::number(bins_.size()-1) + ")!");
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
        THROW(StatisticsException,"Index " + QString::number(index) + " out of range (0-" + QString::number(bins_.size()-1) + ")!");
	}

    return bin_size_*index + min_;
}

double Histogram::binValue(double val, bool as_percentage) const
{
	double value = bins_[binIndex(val)];
	if(as_percentage)
	{
		return 100.0 * value / (double)bin_sum_;
	}
	return value;
}

void Histogram::inc(double val, bool ignore_bounds_errors)
{
	if (ignore_bounds_errors)
	{
		val = BasicStatistics::bound(val, min_, max_);
	}

	bins_[binIndex(val)]+=1;
	bin_sum_ += 1;
}

void Histogram::inc(const QVector<double>& data, bool ignore_bounds_errors)
{
	for (int i=0; i<data.size(); ++i)
	{
		inc(data[i], ignore_bounds_errors);
	}
}


int Histogram::binIndex(double val) const
{
	if (val < min_ || val > max_)
	{
		THROW(StatisticsException, "Requested position '" + QString::number(val) + "' not in range (" + QString::number(min_) + "-" + QString::number(max_) + ")!");
	}

	int index = floor ( (val-min_) / (max_-min_) * bins_.size());
	index = std::max(0, index);
	index = std::min(index, (int)(bins_.size()-1));

	return index;
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

QVector<double> Histogram::xCoords()
{
	return BasicStatistics::range(binCount(), startOfBin(0) + 0.5 * binSize(), binSize());
}

QVector<double> Histogram::yCoords(bool as_percentage)
{
	if (as_percentage)
	{
		QVector<double> tmp(bins_);
		for (int i=0; i<tmp.count(); ++i)
		{
			tmp[i] /= bin_sum_;
		}
		return tmp;
	}
	else
	{
		return bins_;
	}
}
