#include "PlotUtils.h"
#include "Exceptions.h"
#include <QApplication>

PlotUtils::PlotUtils(QChart *chart)
	: chart_(chart)
{
}

void PlotUtils::saveAsPng(QString filename, int w, int h)
{
	// image rendering
	QChartView chartView(chart_);
	chartView.resize(w, h);

	// antialiasing for smoother lines and text
	chartView.setRenderHint(QPainter::Antialiasing, true);
	chartView.setRenderHint(QPainter::TextAntialiasing, true);
	chartView.setRenderHint(QPainter::SmoothPixmapTransform, true);

	QApplication::processEvents();
	QPixmap pixmap = chartView.grab();

	if (!pixmap.save(filename.replace("\\", "/"), "PNG"))
	{
		THROW(ProgrammingException, "Could not save bar plot to file: " + filename);
	}
	delete chart_;
}
