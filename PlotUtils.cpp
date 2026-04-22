#include "PlotUtils.h"
#include "Exceptions.h"
#include <QApplication>
#include <QFontDatabase>
#include <QLineSeries>
#include <QAreaSeries>
#include "Log.h"

PlotUtils::PlotUtils()
	: chart_(new QChart())
{
	// QChart needs an instance of a GUI app and a screen to be rendered, here we make sure it will work properly on the server in a headless mode.
	// QT_QPA_PLATFORM=offscreen environment variables has to be set for the headless mode, otherwise an exception will be thrown
	QCoreApplication* app = QCoreApplication::instance();
	if (!qobject_cast<QApplication*>(app)) THROW(ProgrammingException, "The code needs a running GUI application to be able to render plots");
}

QChart *PlotUtils::getChart()
{		
	return chart_;
}

void PlotUtils::applyFontSettings()
{
	int font_id = QFontDatabase::addApplicationFont(":/fonts/Arimo-Regular.ttf");
	QFontDatabase::addApplicationFont(":/fonts/Arimo-Bold.ttf");
	QFontDatabase::addApplicationFont(":/fonts/Arimo-Medium.ttf");
	QString font_family = QFontDatabase::applicationFontFamilies(font_id).at(0);

	QFont regular_font(font_family, 12);
	regular_font.setWeight(QFont::Normal);

	QFont bold_font(font_family, 12);
	bold_font.setWeight(QFont::Bold);

	chart_->setTitleFont(bold_font);
	for (auto axis : chart_->axes())
	{
		axis->setLabelsFont(regular_font);
		axis->setTitleFont(bold_font);
	}
	chart_->legend()->setFont(regular_font);
}

QFont PlotUtils::getLabelFont()
{
	int font_id = QFontDatabase::addApplicationFont(":/fonts/Arimo-Regular.ttf");
	QString font_family = QFontDatabase::applicationFontFamilies(font_id).at(0);
	return QFont(font_family, 7);
}

void PlotUtils::overpaintAxisX(QValueAxis* axis_x, QValueAxis* axis_y, double max)
{

	// a hack to hide zero-height bars, which are drawn on the top of the x axis
	QLineSeries *upper_x = new QLineSeries();
	QLineSeries *lower_x = new QLineSeries();
	// double max = bars_.size() - 0.5;
	lower_x->append(0, 0);
	upper_x->append(max, 0);
	lower_x->append(max, 0);
	upper_x->append(max, 0);

	QAreaSeries *area_x = new QAreaSeries(upper_x, lower_x);
	area_x->setName("x_axis");

	QColor x_color(axis_x->gridLineColor());
	area_x->setColor(x_color);
	area_x->setBorderColor(x_color);
	chart_->addSeries(area_x);
	area_x->attachAxis(axis_x);
	area_x->attachAxis(axis_y);
}

void PlotUtils::saveAsPng(QString filename, int width, int height)
{
	// setting the font from our resources to maintian consistent rendering across paltforms
	// int id = QFontDatabase::addApplicationFont(":/resources/Arimo-Regular.ttf");
	// QString family = QFontDatabase::applicationFontFamilies(id).at(0);

	// QFont font(family, 12);

	// chart_->setTitleFont(font);

	// for (auto axis : chart_->axes()) {
	// 	axis->setLabelsFont(font);
	// 	axis->setTitleFont(font);
	// }

	// chart_->legend()->setFont(font);


	// image rendering
	QChartView chartView(chart_);
	chartView.resize(width, height);

	// antialiasing for smoother lines and text
	chartView.setRenderHint(QPainter::Antialiasing, true);
	chartView.setRenderHint(QPainter::TextAntialiasing, true);
	chartView.setRenderHint(QPainter::SmoothPixmapTransform, true);

	QApplication::processEvents();
	QPixmap pixmap = chartView.grab();
	pixmap.setDevicePixelRatio(1.0);
	pixmap = pixmap.scaled(width, height, Qt::KeepAspectRatio, Qt::SmoothTransformation);

	if (!pixmap.save(filename.replace("\\", "/"), "PNG"))
	{
		THROW(ProgrammingException, "Could not save bar plot to the file: " + filename);
	}
	delete chart_;
}
