#include <algorithm>
#include <numeric>
#include <cmath>
#include <limits>
#include "BasicStatistics.h"
#include "Exceptions.h"

QVector<double> BasicStatistics::factorial_cache = QVector<double>();

double BasicStatistics::mean(const QVector<double>& data)
{
	const int n = data.count();
	if (n==0)
	{
		THROW(StatisticsException, "Cannot calculate mean on empty data array.");
	}

	return std::accumulate(data.begin(), data.end(), 0.0) / n;
}

double BasicStatistics::stdev(const QVector<double>& data)
{
	return stdev(data, mean(data));
}

double BasicStatistics::stdev(const QVector<double>& data, double mean)
{
	const int n = data.count();
	if (n==0)
	{
		THROW(StatisticsException, "Cannot calculate standard deviation on empty data array.");
	}

	double output = 0.0;
	for (int i=0; i<n; ++i)
	{
		output += pow(data[i]-mean, 2);
	}
	return sqrt(output/n);
}

double BasicStatistics::median(const QVector<double>& data, bool check_sorted)
{
	if (check_sorted && !isSorted(data))
	{
		THROW(StatisticsException, "Cannot calculate median on unsorted data array.");
	}
	const int n = data.count();
	if (n==0)
	{
		THROW(StatisticsException, "Cannot calculate median on empty data array!");
	}

	if (n%2==0)
	{
		return 0.5 * (data[n/2] + data[n/2-1]);
	}
	else
	{
		return data[n/2];
	}
}

double BasicStatistics::mad(const QVector<double>& data, double median)
{
	QVector<double> devs;
	devs.reserve(data.count());
	foreach(double value, data)
	{
		devs.append(fabs(value-median));
	}
	std::sort(devs.begin(), devs.end());
	return BasicStatistics::median(devs, false);
}

double BasicStatistics::q1(const QVector<double>& data, bool check_sorted)
{
	if (check_sorted && !isSorted(data))
	{
		THROW(StatisticsException, "Cannot calculate q1 on unsorted data array.");
	}
	const int n = data.count();
	if (n==0)
	{
		THROW(StatisticsException, "Cannot calculate q1 on empty data array!");
	}

	return data[n/4];
}

double BasicStatistics::q3(const QVector<double>& data, bool check_sorted)
{
	if (check_sorted && !isSorted(data))
	{
		THROW(StatisticsException, "Cannot calculate q3 on unsorted data array.");
	}
	const int n = data.count();
	if (n==0)
	{
		THROW(StatisticsException, "Cannot calculate q3 on empty data array!");
	}

	return data[3*n/4];
}

double BasicStatistics::correlation(const QVector<double>& x, const QVector<double>& y)
{
	if (x.count()!=y.count())
	{
		THROW(StatisticsException, "Cannot calculate correlation of data arrays with different length!");
	}
	if (x.count()==0)
	{
		THROW(StatisticsException, "Cannot calculate correlation of data arrays with zero length!");
	}

	const double x_mean = mean(x);
	const double y_mean = mean(y);

	double sum = 0.0;
	for(int i=0; i<x.size(); ++i)
	{
		sum += (x[i]-x_mean) * (y[i]-y_mean);
	}

	return sum / stdev(x, x_mean) / stdev(y, y_mean) / x.size();
}

bool BasicStatistics::isValidFloat(double value)
{
	if (value != value)
	{
		return false;
	}
	if (value > std::numeric_limits<double>::max())
	{
		return false;
	}
	if (value < -std::numeric_limits<double>::max())
	{
		return false;
	}

	return true;
}

bool BasicStatistics::isValidFloat(QByteArray value)
{
	bool ok = true;
	double numeric_value = value.toDouble(&ok);
	return ok && isValidFloat(numeric_value);
}

bool BasicStatistics::isSorted(const QVector<double>& data)
{
	for (int i=1; i<data.count(); ++i)
	{
		if (data[i-1]>data[i]) return false;
	}

	return true;
}

int BasicStatistics::sign(int val)
{
	return (0<val) - (val<0);
}

QPair<double, double> BasicStatistics::linearRegression(const QVector<double>& x, const QVector<double>& y)
{
	// initializing sum of x and y values
	int count_valid = 0;
	double sum_x = 0.0;
	double sum_y = 0.0;
	for (int i=0; i<x.size(); ++i)
	{
		if (isValidFloat(x[i]) && isValidFloat(y[i]))
		{
			sum_x += x[i];
			sum_y += y[i];
			++count_valid;
		}
	}

	// middle index of the section
	double sxoss = sum_x / count_valid;

	// initializing b
	double slope = 0.0;
	double st2 = 0.0;
	for (int i=0; i<x.size(); ++i)
	{
		if (isValidFloat(x[i]) && isValidFloat(y[i]))
		{
			// t is distance from the middle index
			double t = x[i]-sxoss;

			// sum of the squares of the distance from the average
			st2 += t*t;

			// b is sum of datapoints weighted by the distance
			slope += t * y[i];
		}
	}

	// averaging b by the maximum distance from the average
	slope /= st2;

	// average difference between data points and distance weighted data points.
	double offset = (sum_y - sum_x*slope) / count_valid;

	//create output
	return qMakePair(offset, slope);
}

QPair<double, double> BasicStatistics::getMinMax(const QVector<double>& data)
{
	double min = std::numeric_limits<double>::max();
	double max = -std::numeric_limits<double>::max();
	foreach(double v, data)
	{
		if (!isValidFloat(v)) continue;

		min = std::min(min, v);
		max = std::max(max, v);
	}

	return qMakePair(min, max);
}

void BasicStatistics::precalculateFactorials()
{
	if (!factorial_cache.isEmpty()) return;

	//calculate factorials until double overflow happens
	int i = 0;
	double value = 1.0;
	while(isValidFloat(value))
	{
		factorial_cache.append(value);
		++i;
		value *= i;
	}
}

double BasicStatistics::factorial(int n)
{
	//outside of valid ragen => exception
	if (n<0 || factorial_cache.count()==0)
	{
		THROW(ProgrammingException, "Cannot calculate factorial of " + QByteArray::number(n) + "! Cache not initialized?");
	}

	//not in cache (i.e. double overflow) => NAN
	if (factorial_cache.count()<n+1)
	{
		return std::numeric_limits<double>::quiet_NaN();
	}

	return factorial_cache[n];
}

double BasicStatistics::matchProbability(double p, int n, int count)
{
	//handle double overflow of factorial (approximately at 160)
	int mismatches = count - n;
	while(!BasicStatistics::isValidFloat(BasicStatistics::factorial(count)))
	{
		n /= 2;
		mismatches /=2;
		count = n + mismatches;
	}

	//calculate probability
	double output = 0.0;
	for (int i=n; i<=count; ++i)
	{
		double q = std::pow(1.0-p, count-i) * std::pow(p, i) * BasicStatistics::factorial(count) / BasicStatistics::factorial(i) / BasicStatistics::factorial(count-i);
		output += q;
	}

	//check that result is valid
	if (!BasicStatistics::isValidFloat(output))
	{
		THROW(ProgrammingException, "Calculated probabilty for " + QString::number(n) + " matches and " + QString::number(mismatches) + " mismatches is not a valid float!");
	}

	return output;
}
