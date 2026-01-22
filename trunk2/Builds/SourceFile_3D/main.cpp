#include <windows.h>
#include "OSGEarthApp.h"
#include <QTextCodec>
#include <QDesktopWidget>
#include <QTextCodec>

#include <osgEarth/Map>
#include <osgEarth/Registry>
#include <osgViewer/Viewer>

int main(int argc, char* argv[])
{
	QApplication a(argc, argv);
	//ÉèÖÃ±àÂë
	QTextCodec* codec = QTextCodec::codecForLocale();
	QTextCodec::setCodecForLocale(codec);
	setlocale(LC_ALL, "chs");

	OSGEarthApp w;
	w.setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint | Qt::WindowMinimizeButtonHint);

	QRect deskRect = QApplication::desktop()->availableGeometry();
	QRect screenRect = QApplication::desktop()->screenGeometry();
	int scrWidth = screenRect.width();
	int scrHeight = screenRect.height();
	if (scrWidth > 1600 && scrHeight > 900)
	{
		w.move((scrWidth - 1600) / 2, (scrHeight - 900 - 40) / 2);
	}
	w.show();
	return a.exec();
}

