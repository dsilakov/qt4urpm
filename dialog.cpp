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

#include "dialog.h"
#include "qt4urpm.h"
#include <QMessageBox>
#include <QPixmap>

Dialog::Dialog(QString message)
{
	// let us be paranoid - if something behaves wrong, the dialog will at least
	// return actioncanceled = true
	actioncanceled = true;

	QVBoxLayout * vbox   = new QVBoxLayout;
	QHBoxLayout * hbox   = new QHBoxLayout;
	QVBoxLayout * AllBox = new QVBoxLayout;

	QTextEdit * dialogtext = new QTextEdit;

	dialogtext->setText(message);
	dialogtext->setReadOnly(true);

	QDialogButtonBox * buttonBox = new QDialogButtonBox(QDialogButtonBox::Yes
	                                                   |QDialogButtonBox::No);

	connect(buttonBox, SIGNAL(accepted()), this, SLOT(close()));
	connect(buttonBox, SIGNAL(accepted()), this, SLOT(okAct()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(cancelAct()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(close()));

	vbox->addWidget(dialogtext);
	hbox->addStretch();
	hbox->addWidget(buttonBox);
	hbox->addStretch();
	AllBox->addLayout(vbox);
	AllBox->addLayout(hbox);

	setMinimumWidth(500);
	setMinimumHeight(260);
	setLayout(AllBox);
}

CheckBoxDialog::CheckBoxDialog(QString & msg, QString & cbmsg)
{
	// let us be paranoid - if something behaves wrong, the dialog will at least
	// return actioncanceled = true
	actioncanceled = true;
	showWarning = true;

	QHBoxLayout * hwarn  = new QHBoxLayout;
	QVBoxLayout * vbox   = new QVBoxLayout;
	QHBoxLayout * hbox   = new QHBoxLayout;
	QVBoxLayout * AllBox = new QVBoxLayout;

	QPixmap warnPiMa(WARNICON);
	QLabel * warnIcon = new QLabel();
	warnIcon->setPixmap(warnPiMa);

	QTextEdit * dialogtext = new QTextEdit;

	showWarningCB = new QCheckBox(cbmsg, this);
	showWarningCB->setChecked(false);

	dialogtext->setText(msg);
	dialogtext->setReadOnly(true);

	QDialogButtonBox * buttonBox = new QDialogButtonBox(QDialogButtonBox::Yes
	                                                   |QDialogButtonBox::No);

	connect(buttonBox, SIGNAL(accepted()), this, SLOT(okAct()));
	connect(buttonBox, SIGNAL(accepted()), this, SLOT(preclose()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(cancelAct()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(preclose()));

	hwarn->addWidget(dialogtext);
	hwarn->addWidget(warnIcon);
	vbox->addWidget(showWarningCB);
	hbox->addStretch();
	hbox->addWidget(buttonBox);
	hbox->addStretch();
	AllBox->addLayout(hwarn);
	AllBox->addLayout(vbox);
	AllBox->addLayout(hbox);

	setMinimumWidth(500);
	setMinimumHeight(260);
	setLayout(AllBox);
}

void CheckBoxDialog::preclose() {
	// Write CheckBoxState to showWarning
	showWarning = !showWarningCB->isChecked();

	// Now really close
	close();
}
