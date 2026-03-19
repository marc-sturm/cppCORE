#include "PlotUtils.h"
#include "Exceptions.h"
#include <QApplication>

PlotUtils::PlotUtils()
{
}

QChart *PlotUtils::createEmptyChart()
{
	// QChart needs an instance of a GUI app and a screen to be rendered, here we make sure it will work properly on the server in a headless mode.
	// QT_QPA_PLATFORM=offscreen environment variables has to be set for the headless mode, otherwise an exception will be thrown
	QCoreApplication* app = QCoreApplication::instance();
	if (!qobject_cast<QApplication*>(app)) THROW(ProgrammingException, "The code needs a running GUI application to be able to render plots");
	chart_ = new QChart();
	return chart_;
}

void PlotUtils::saveAsPng(QString filename, int width, int height)
{
	// image rendering
	QChartView chartView(chart_);
	chartView.resize(width, height);

	// antialiasing for smoother lines and text
	chartView.setRenderHint(QPainter::Antialiasing, true);
	chartView.setRenderHint(QPainter::TextAntialiasing, true);
	chartView.setRenderHint(QPainter::SmoothPixmapTransform, true);

	QApplication::processEvents();
	QPixmap pixmap = chartView.grab();

	if (!pixmap.save(filename.replace("\\", "/"), "PNG"))
	{
		THROW(ProgrammingException, "Could not save bar plot to the file: " + filename);
	}
	delete chart_;
}
