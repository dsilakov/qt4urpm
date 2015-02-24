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

#ifndef DIALOG_H
#define DIALOG_H
#include <QtGui>
#include <QCheckBox>

namespace Ui
{
	class qt4urpm;
}

/// shows a dialog with a text and "ok" and "cancel" buttons
class Dialog : public QDialog
{
	Q_OBJECT
public:
	Dialog (QString);

public slots:
	void cancelAct() {actioncanceled = true;}
	void okAct()     {actioncanceled = false;}
	bool getStatus() {return actioncanceled;}

private:
	bool actioncanceled;
};

class CheckBoxDialog : public QDialog
{
	Q_OBJECT
public:
	CheckBoxDialog (QString & msg, QString & cbmsg);

	bool getStatus()      {return actioncanceled;}
	bool getShowWarning() {return showWarning;}

public slots:
	void cancelAct() {actioncanceled = true;}
	void okAct()     {actioncanceled = false;}
	void preclose();

private:
	bool        actioncanceled;
	bool        showWarning;
	QCheckBox * showWarningCB;
};

#endif // DIALOG_H
