// Copyright Carl Philipp Klemm 2026
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QApplication>
#include <QThread>

#include "MainWindow.h"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	MainWindow window;
	window.show();

	return app.exec();
}
