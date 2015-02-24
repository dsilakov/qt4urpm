/*
	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <QtGui/QApplication>
#include <QAction>
#include <QDesktopServices>
#include <QSettings>
#include "qt4urpm.h"
#include <string.h>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	QString locale = QLocale::system().name();
	QTranslator generalTranslator;
	QTranslator qt4urpmTranslator;
	QString general(getenv("QTDIR"));
	general += "/translations/";
	generalTranslator.load("qt_" + locale, general);
	qt4urpmTranslator.load("qt4urpm_" + locale, LOCALEPATH);
	a.installTranslator(&generalTranslator);
	a.installTranslator(&qt4urpmTranslator);
	// Check whether user == root
	if (getuid() != 0 )
	{
		// Show warning message, as long as user did not disable it.
		QSettings config("MandrivaUser.de", "qt4urpm");
		if (config.value("qt4urpm/showWaring", true).toBool()) {
			// Show pre start warning
			QString buf
				(QMainWindow::tr("This tool works system wide. Only use it if you "
				                 "know what you are doing! It is possible, that "
				                 "you damage or even destroy your system! Go on "
				                 "anyways?"));
			QString buf2
				(QMainWindow::tr("Do not show this warning again!"));

			CheckBoxDialog warndia(buf, buf2);
			warndia.setAttribute(Qt::WA_QuitOnClose);
			warndia.setWindowTitle(QMainWindow::tr("Warning!"));
			warndia.exec();

			// Set the value of the checkbox
			config.setValue("qt4urpm/showWaring", warndia.getShowWarning());

			// Do nothing, if user canceled
			if (warndia.getStatus()) {
				return 0;
			}
		}

		// Try to start as root
		qt4urpm::termlog("Trying to start as root.");
		std::string cmd("/usr/lib/kde4/libexec/kdesu ");
		cmd += argv[0];
		if (system(cmd.c_str()) != 0) {
			cmd = "/usr/lib64/kde4/libexec/kdesu ";
			cmd += argv[0];
			if (system(cmd.c_str()) != 0) {
				QString msg
					(QMainWindow::tr("You need to be root!\n\nTo ease the use of "
					 "qt4urpm we suggest to install libkdesu5 (run \"gurpmi libkdesu5\")"));
				QMessageBox msgBox
					(QMessageBox::Critical, QMainWindow::tr("Not root!"), msg);
				msgBox.exec();
			}
		}
		return 0;
	}

	// Start up the main application
	qt4urpm w;
	w.show();
	return a.exec();
}
