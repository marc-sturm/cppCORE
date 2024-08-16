#ifndef BASICSTATISTICS_H
#define BASICSTATISTICS_H

#include "cppCORE_global.h"
#include <QVector>
#include <QPair>

///Statistics helper class.
class CPPCORESHARED_EXPORT BasicStatistics
{
public:
	///Calculates the mean of the data.
	static double mean(const QVector<double>& data);
	///Calculates the mean of the data in sub-range of indices.
	static double mean(const QVector<double>& data, int start_index, int end_index);
	///Calculates the standard deviation of the data.
	static double stdev(const QVector<double>& data);
	///Calculates the standard deviation of the data, using a given mean.
	static double stdev(const QVector<double>& data, double mean);
	///Calculates the standard deviation of the data, using a given mean in sub-range of indices.
	static double stdev(const QVector<double>& data, double mean, int start_index, int end_index);
	///Calculates the median from sorted data.
	static double median(const QVector<double>& data, bool check_sorted=false);
	///Calculates the median average deviation (multiply by 1.482 to get a robust estimator of the stdev).
	static double mad(const QVector<double>& data, double median);
	///Calculates the first quartile from sorted data.
	static double q1(const QVector<double>& data, bool check_sorted=false);
	///Calculates the third quartile from sorted data.
	static double q3(const QVector<double>& data, bool check_sorted=false);
	///Calculates the correlation of two data arrays.
	static double correlation(const QVector<double>& x, const QVector<double>& y);
	///Calculates the correlation of two data arrays in sub-range of indices.
	static double correlation(const QVector<double>& x, const QVector<double>& y, int start_index, int end_index);

	///Returns if a float is valid.
	static bool isValidFloat(double value);
	///Returns if a string is a representations of a valid float.
	static bool isValidFloat(QByteArray value);
	///Returns the value bounded to the given range.
	template <typename T>
	static T bound(T value, T lower_bound, T upper_bound)
	{
		if (value<lower_bound) return lower_bound;
		if (value>upper_bound) return upper_bound;
		return value;
	}

	///Returns if the data is sorted.
	static bool isSorted(const QVector<double>& data);
	///Returns the sign of an integer, or 0 for 0.
	static int sign(int val);

	///Returns the offset and slope of a linear regression. Ignores invalid values.
	static QPair<double, double> linearRegression(const QVector<double>& x, const QVector<double>& y);
	///Returns minimum and maximum of a dataset. Ignores invalid values.
	static QPair<double, double> getMinMax(const QVector<double>& data);

	///Returns an even-spaced range of values.
	template <typename T>
	static QVector<T> range(int size, T start_value, T increment)
	{
		T next_val = start_value;

		QVector<T> output;
		output.reserve(size);
		while(output.count()<size)
		{
			output << next_val;
			next_val += increment;
		}
		return output;
	}

	///Precalculates factorials.
	static void precalculateFactorials();

	///Returns the factorial of 'n', or 'nan' if an overflow happened.
	static double factorial(int n);

	///Returns the probability to see 'n' or more matches in 'count' observation when the probability to see a single match is 'p' (via binomial distribution)
	static double matchProbability(double p, int n, int count);

	///Returns if two ranges overlap. Coordinates are 1-based.
	static bool rangeOverlaps(int start1, int end1, int start2, int end2)
	{
		return start1<=end2 && end1>=start2;
	}

	///Precalculates log factorials.
	static void precalculateLogFactorials();

	/// Returns the log factorial of 'n'
	static double logFactorial(int n);

	///Returns the log hypergeometric probability (required for Fisher's Exact Test)
	static double hypergeometricLogProbability(int a, int b, int c, int d);

	///Returns the p-value of a two-sided Fisher's Exact Test
	/// based on https://genome.sph.umich.edu/w/images/b/b3/Bios615-fa12-lec03-presentation.pdf
	static double fishersExactTest(int a, int b, int c, int d, QByteArray type);


protected:
	static QVector<double> log_factorial_cache;
	static QVector<double> factorial_cache;

};

#endif
