#pragma once
#include <QScreen>
#include <QFile>
#include <QApplication>

static double getReduce()
{
	QScreen* screen = qApp->primaryScreen();
	qreal    dotsPerInch = screen->logicalDotsPerInch();
	double   screenScale = (double)dotsPerInch / 96.0;

	double reduce = 1.0;
	if (screenScale < 1.01)
		reduce = 1.0;
	else if (screenScale < 1.26)
		reduce = 0.98;
	else if (screenScale < 1.51)
		reduce = 0.96;
	else if (screenScale < 1.76)
		reduce = 0.94;
	else if (screenScale < 2.01)
		reduce = 0.92;
	else
		reduce = 0.9;
	return reduce;
}

static int getBottomPix()
{
	QScreen* screen = qApp->primaryScreen();
	qreal    dotsPerInch = screen->logicalDotsPerInch();
	double   screenScale = (double)dotsPerInch / 96.0;

	int btmPix = 40;
	if (screenScale < 1.01)
		btmPix = 40;
	else if (screenScale < 1.26)
		btmPix = 50;
	else if (screenScale < 1.51)
		btmPix = 60;
	else if (screenScale < 1.76)
		btmPix = 70;
	else if (screenScale < 2.01)
		btmPix = 80;
	else
		btmPix = 90;
	return btmPix;
}
static int getFontSize(int ori)
{
	QScreen* screen = qApp->primaryScreen();
	qreal    dotsPerInch = screen->logicalDotsPerInch();
	double   screenScale = (double)dotsPerInch / 96.0;

	int fontSize = ori;
	if (screenScale < 1.01)
		fontSize = ori;
	else if (screenScale < 1.26)
		fontSize = ori * 0.80;
	else if (screenScale < 1.51)
		fontSize = ori * 0.74;
	else if (screenScale < 1.76)
		fontSize = ori * 0.62;
	else if (screenScale < 2.01)
		fontSize = ori * 0.5;
	else
		fontSize = ori * 0.45;
	return fontSize;
}

static QString getStyleSheet(QString control,int ori)
{
	QString cssPath = QString("%1/qss/%2.css").arg(QApplication::applicationDirPath()).arg(control);
	QFile file(cssPath);
	//打开json文件并判断（不成功则返回0）
	if (!file.open(QIODevice::ReadOnly))
		return "";
	//将文件内容读取到数组中
	QByteArray bycss(file.readAll());
	file.close();
	QString strcss = bycss;
	QString css = strcss.replace("%1",QString::number(getFontSize(ori)));
	css = css.replace("%2", QApplication::applicationDirPath());
	return css;
}