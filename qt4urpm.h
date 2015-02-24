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

#ifndef QT4URPMI_H
#define QT4URPMI_H

// Version definition
#define QT4URPMI_VERSION "1.0.1"
#define COPYRIGHT        "Copyright &copy; TeaAge, Nasenbaer 2009-2011"
#define HOMEPAGE         "http://sf.net/projects/qt4urpm"
#define WARNICON         "/usr/share/icons/oxygen/128x128/status/security-low.png"

// For localisation
#ifndef LOCALEPATH
#define LOCALEPATH       "./"
#endif


#include <QMainWindow>
#include <QtCore>
#include <QtGui>
#include <QVector>
#include <iostream>
#include <unistd.h>
#include "dialog.h"

enum {
    LISTTRANSACTIONS,
    LISTTRANSACTIONSHANDLED,
	ORPHANSEARCH,
	ORPHANSEARCHHANDLED,
	PACKAGESEARCH,
	PACKAGESEARCHHANDLED,
	PACKAGEDEPENDENCIES,
	PACKAGEDEPENDENCIESHANDLED,
	NONE
};

namespace Ui
{
	class qt4urpm;
}

class qt4urpm : public QMainWindow
{
	Q_OBJECT

public:
	qt4urpm(QWidget * parent = 0);
	~qt4urpm();

public slots:
    void list_transactions();
	void orphansearch();
	void packagesearch();
	void processData();
	void procfinish();
	void showHelp();
	void showAbout();
	void showAboutQt();
	void notOrphAct() {prepareAction(false);}
	void removeAct()  {prepareAction(true);}
	void installAct() {install();}
	void buttonAct();
	void changeTooltip(int r, int c);
    void rollback();

	/// standardized terminal output
	static void termlog(std::string const str)
		{std::cout << "Qt4urpm:  " << str << "\n";}

private:
	void prepareAction(bool);
	void install();
	void notOrphaned();
	void remove();
	QString toUList(QString);

private:
	Ui::qt4urpm      * ui;

	QProcess           proc;
	QProcess           sumproc;
	QProcess           grepproc;
	int                procmode;

	QVector<QString>   orphans;
    QVector<QString>   transactions;
	QString            selectedOrphans;
	QString            dependencies;
	QString            selectedInstall;
	Dialog           * mydialog;

    char             * procout;

};

#endif // QT4URPMI_H
