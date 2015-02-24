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


#include "qt4urpm.h"
#include "dialog.h"
#include "ui_qt4urpm.h"
#include <QAction>
#include <QDesktopServices>
#include <QMouseEvent>
#include <QTemporaryFile>

using namespace std;


/// constructor
qt4urpm::qt4urpm(QWidget *parent)
: QMainWindow(parent), ui(new Ui::qt4urpm), procmode(NONE)
{
	ui->setupUi(this);
	ui->orph_results->setColumnWidth(0,505);
	ui->find_results->setColumnWidth(0,505);
    connect(ui->trs_update,       SIGNAL(clicked()), this, SLOT(list_transactions()));
    connect(ui->trs_rollback,     SIGNAL(clicked()), this, SLOT(rollback()));
	connect(ui->orph_search,      SIGNAL(clicked()), this, SLOT(orphansearch()));
	connect(ui->orph_notOrphaned, SIGNAL(clicked()), this, SLOT(notOrphAct()));
	connect(ui->orph_remove,      SIGNAL(clicked()), this, SLOT(removeAct()));
	connect(ui->find_search,      SIGNAL(clicked()), this, SLOT(packagesearch()));
	connect(ui->find_install,     SIGNAL(clicked()), this, SLOT(installAct()));
	connect(ui->find_lineEdit,    SIGNAL(textEdited(const QString &)), this, SLOT(buttonAct()));
	connect(ui->find_lineEdit,    SIGNAL(returnPressed ()), this, SLOT(packagesearch()));
	connect(&proc, SIGNAL(readyReadStandardOutput()), this, SLOT(processData()));
	connect(&proc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(procfinish()));
	connect(ui->orph_results, SIGNAL(cellEntered(int,int)), this, SLOT(changeTooltip(int,int)));

    procout = tmpnam(NULL);

    // Let's fill the lists
	orphansearch();
    list_transactions();
}

/// destructor
qt4urpm::~qt4urpm()
{
	// Better safer than safe
	if (ui)
		delete ui;

    QFile::remove(procout);
}

/**
 * orphansearch()
 * searches for orphaned packages
 */
void qt4urpm::orphansearch()
{
	if (procmode != NONE)
		proc.close();
	QString command("urpmq --auto-orphans");
	termlog(command.toStdString());
	ui->urpm_status->setText(tr("Collecting data ..."));
	procmode = ORPHANSEARCH;
    proc.setStandardOutputFile(procout);
	proc.start(command);
    proc.waitForFinished(-1);
    processData();
    procmode = NONE;
}

/**
 * list_transactions()
 * list transactions that can be reverted
 */
void qt4urpm::list_transactions()
{
    if (procmode != NONE) {
        proc.close();
    }
//    QString command("find /var/spool/repackage -type d | sort -n | head -2 | tail -1 | while read a; do b=`basename $a`; ((b=$b-1)); date --date @$b \"+%Y-%m-%d %H:%M:%S\"; done | while read last_date; do urpmi.recover --list \"$last_date\"; done");
    QString command("urpmi.recover --list-safe");
    termlog(command.toStdString());
    ui->urpm_status->setText(tr("Collecting data ..."));
    procmode = LISTTRANSACTIONS;
    proc.setStandardOutputFile(procout);
	proc.start(command);
    proc.waitForFinished(-1);
    processData();
    procmode = NONE;
}

/**
* packagesearch()
* searches for packages containing the file specified in find_lineEdit
*/
void qt4urpm::packagesearch()
{
	// Clean up the table
	ui->find_results->setRowCount(0);

	// Only search if find_lineEdit holds some text
	if (!ui->find_lineEdit->text().size())
		return;

	if (procmode != NONE)
		proc.close();
	QString command("urpmf ");
	command += ui->find_lineEdit->text();
	termlog(command.toStdString());
	ui->urpm_status->setText(tr("Collecting data ..."));
	procmode = PACKAGESEARCH;
    proc.setStandardOutputFile(procout);
	proc.start(command);
    proc.waitForFinished(-1);
    processData();
    procmode = NONE;
}

/// processes the output of a proc.start()
void qt4urpm::processData()
{
    QString trans_date;
    QString trans_pkgs;
    int i, row;

    QFile file(procout);
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }

    QTextStream in(&file);

	switch (procmode) {
        case LISTTRANSACTIONS:
            // Cleanup
            while (transactions.size())
                transactions.pop_back();

            // Begin of processing the data
            termlog("Processing ...");
            ui->urpm_status->setText(tr("Processing data ..."));

            row=0;
            i=0;

            while (!in.atEnd()) {
                QString newtrans = in.readLine();
                // Skip first two lines
                i++;
                if (i <= 2) {
                    continue;
                }
                newtrans = newtrans.trimmed();
                QStringList trans_elems = newtrans.split(" ");

                // Launch urpmi.recover --list-safe to get an idea of how its output looks like.
                // Here we want to combine all packages correcponding to the same date in one cell.
                if (trans_elems.length() > 2) {
                    if (!trans_date.isEmpty()) {
                        // Dump previously collected entries
                        ui->recover_list_transactions->setRowCount(row+1);
                        transactions.push_back(trans_date);
                        ui->recover_list_transactions->setItem(row, 0, new QTableWidgetItem(trans_date));
                        ui->recover_list_transactions->setItem(row, 1, new QTableWidgetItem(trans_pkgs));
                        // Do not allow the user to change the line - just selection.
                        ui->recover_list_transactions->item(row, 0)->setFlags
                            (Qt::ItemIsEnabled | Qt::ItemIsSelectable);
                        ui->recover_list_transactions->item(row, 1)->setFlags
                            (Qt::ItemIsEnabled | Qt::ItemIsSelectable);
                        row++;
                    }

                    trans_date = trans_elems[0] + " " + trans_elems[1];
                    trans_pkgs = trans_elems[2];
                    if (trans_elems.length() > 3) {
                        trans_pkgs += " " + trans_elems[3];
                    }
                    continue;
                }
                else {
                    trans_pkgs += "\n" + trans_elems[0];
                    if (trans_elems.length() > 1) {
                        trans_pkgs += " " + trans_elems[1];
                    }
                    continue;
                }

            }

            // Dump the last record
            if (!trans_date.isEmpty()) {
                // Dump previously collected entries
                ui->recover_list_transactions->setRowCount(row+1);
                transactions.push_back(trans_date);
                ui->recover_list_transactions->setItem(row, 0, new QTableWidgetItem(trans_date));
                ui->recover_list_transactions->setItem(row, 1, new QTableWidgetItem(trans_pkgs));
                // Do not allow the user to change the line - just selection.
                ui->recover_list_transactions->item(row, 0)->setFlags
                    (Qt::ItemIsEnabled | Qt::ItemIsSelectable);
                ui->recover_list_transactions->item(row, 1)->setFlags
                    (Qt::ItemIsEnabled | Qt::ItemIsSelectable);
                row++;
            }

            ui->urpm_status->setText(tr("Processing of data completed."));
            procmode = LISTTRANSACTIONSHANDLED;
            break;
		case ORPHANSEARCH:
			// Cleanup
			while (orphans.size())
				orphans.pop_back();

			// Begin of processing the data
			termlog("Processing ...");
			ui->urpm_status->setText(tr("Processing data ..."));
            i=0;
            while (!in.atEnd()) {
                QString neworph = in.readLine();
				ui->orph_results->setRowCount(i+1);
				orphans.push_back(neworph);
				ui->orph_results->setItem(i, 0, new QTableWidgetItem(neworph));
				// Do not allow the user to change the line - just selection.
				ui->orph_results->item(i, 0)->setFlags
					(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
                i++;
			}
			ui->urpm_status->setText(tr("Processing of data completed."));
			procmode = ORPHANSEARCHHANDLED;
			break;
		case PACKAGESEARCH:
			// Begin of processing the data
			{
				termlog("Processing ...");
				ui->urpm_status->setText(tr("Processing data ..."));
                i=0;
                while (!in.atEnd()) {
                    QString newres = in.readLine();
					ui->find_results->setRowCount(i+1);
					ui->find_results->setItem(i, 0, new QTableWidgetItem(newres));
					// Do not allow the user to change the line - just selection.
					ui->find_results->item(i, 0)->setFlags
						(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
                    i++;
				}
				ui->urpm_status->setText(tr("Processing of data completed."));
				procmode = ORPHANSEARCHHANDLED;
			}
			break;
		case PACKAGEDEPENDENCIES:
			// Cleanup
			dependencies = "";

			// Begin of processing the dependencies
			termlog("Processing dependencies...");
			ui->urpm_status->setText(tr("Processing dependencies data ..."));
            i=0;
            while (!in.atEnd()) {
                QString dep = in.readLine();
				if ((orphans.indexOf(dep) == -1) || (selectedOrphans.indexOf(" " + dep) > -1))
					// either not installed or already in remove list
					continue;
				dependencies += " ";
				dependencies += dep;
                i++;
			}
			ui->urpm_status->setText(tr("Processing of data completed."));
			procmode = PACKAGEDEPENDENCIESHANDLED;
			return remove();
		default:
			return;
	}
}

/// called once proc was finished
void qt4urpm::procfinish()
{
	proc.close();
	switch(procmode) {
        case LISTTRANSACTIONS:
            // Clean up the table - urpmq.recover --list did not return any transactions that can be reverted
            ui->recover_list_transactions->setRowCount(0);
            ui->urpm_status->setText(tr("No transactions to rollback found."));
            break;
		case ORPHANSEARCH:
			// Clean up the table - urpmq --auto-orphans did not find anything
			ui->orph_results->setRowCount(0);
			ui->urpm_status->setText(tr("No orphaned packages found."));
			break;
		case PACKAGESEARCH:
			// urpmf did not find anything
			ui->urpm_status->setText(tr("No packages found."));
			break;
		case PACKAGEDEPENDENCIES:
			// normally never here - however it seems as if the package does not
			// even depend on itself
			procmode = NONE;
			remove();
			return;
		default:
			break;
	}
    //procmode = NONE;
}

/// prepares the data for a package action - e.g. searches for dependencies.
void qt4urpm::prepareAction(bool remove)
{
	// Only make buttons usable, if something is selected
	if (ui->orph_results->selectedItems().empty())
		return;

	selectedOrphans = "";
	QList<QTableWidgetItem *> packagelist(ui->orph_results->selectedItems());

	// fill string "selectedOrphans" with all selected packages
	for (int i = 0; i < packagelist.size(); ++i) {
		selectedOrphans += " ";
		selectedOrphans += packagelist.at(i)->data(Qt::DisplayRole).toString();
	}
	if (remove) { // search for dependening packages
		if (procmode != NONE)
			proc.close();
		procmode = PACKAGEDEPENDENCIES;
		QString command("urpmq --whatrequires-recursive ");
		command += selectedOrphans;
		command.remove('\n');
		termlog(command.toStdString());
		ui->urpm_status->setText(tr("Checking dependencies ..."));
		proc.start(command);
	} else // start not orphaned action
		notOrphaned();
}

/// installs the selected packages and dependencies
void qt4urpm::install()
{
	// Cleanup
	selectedInstall = "";

	// fill string "selectedInstall" with all selected packages
	QString newres;
	QVector<QString> check;
	QList<QTableWidgetItem *> packagelist(ui->find_results->selectedItems());
	for (int i = 0; i < packagelist.size(); ++i) {
		newres = packagelist.at(i)->data(Qt::DisplayRole).toString();
		newres.resize(newres.indexOf(':')); // we just need the package
		if (check.indexOf(newres) != -1) // each package only once.
			continue;
		check.push_back(newres);
		selectedInstall += " ";
		selectedInstall += newres;
		selectedInstall += "\n"; // This time we add it ourselves for the output.
	}

	// Install selected packages - we simply use gurpmi here.
	// There is no need to reinvent the wheel ;-)
	QString command("gurpmi ");
	command += selectedInstall;
	if(system(command.toStdString().c_str()) != 0) {
		termlog("An error happend during command execution.");
		ui->urpm_status->setText(tr("WARNING: Error during execution!"));
	} else {
		termlog("Installed:" + selectedInstall.toStdString());
		ui->urpm_status->setText(
			tr("Packages successfully installed!"));
	}
}

/**
 * notOrphaned()
 * marks the the selected package as not orphaned via urpmi
 */
void qt4urpm::notOrphaned()
{
	string cmd("/usr/sbin/urpmi");
	QList<QTableWidgetItem *> packagelist(ui->orph_results->selectedItems());

	// Show a dialog to ask the user whether he/she really wants to mark the
	// packages "cmd" as not orphaned
	QString buf("<qt><p><b>");
	buf += tr("You are about to do a system wide change!");
	buf += "<br><br><i>";
	buf += tr("Selected packages:");
	buf += "</i></b></p><p>";
	buf += toUList(selectedOrphans);
	buf += "</p><p><b>";
	buf += tr("Are you sure you want to mark these packages as not orphaned?");
	buf += "</b></p></qt>";

	mydialog = new Dialog(buf);
	mydialog->setAttribute(Qt::WA_QuitOnClose);
	mydialog->setWindowTitle(tr("Warning!"));
	mydialog->exec();

	// Do nothing, if user canceled
	if (mydialog->getStatus()) {
		ui->urpm_status->setText(tr("Action canceled by user!"));
		return;
	}

	// Remove the '\n' qt adds
	selectedOrphans.remove('\n');

	// Finally "install" the packages (== mark as not orphaned)
	cmd += selectedOrphans.toStdString();
	termlog(cmd);
	if (system(cmd.c_str()) != 0) {
		orphansearch();
		termlog("An error happend during command execution.");
		ui->urpm_status->setText(tr("WARNING: Error during execution!"));
	} else {
		orphansearch();
		termlog("Marked as not orphaned:" + selectedOrphans.toStdString());
		ui->urpm_status->setText(
			tr("Packages successfully marked as not orphaned!"));
	}
}

/**
 * rollback()
 * rollbacks the package base to a given date
 */
void qt4urpm::rollback()
{
    string cmd("/usr/sbin/urpmi.recover --auto --rollback ");
    QList<QTableWidgetItem *> packagelist(ui->recover_list_transactions->selectedItems());

    if (packagelist.size() == 0) {
        ui->urpm_status->setText(tr("No transaction selected"));
        return;
    }

    QString targetTr = packagelist.at(0)->data(Qt::DisplayRole).toString();

    // Provide user with the info - which packages will be
    // removed and downgraded
    // Show a dialog to ask the user whether he/she really wants to rollback
    // to specified checkpoint
    QString buf("<qt><p><b>");
    buf += tr("You are about to do a system wide change!");
    buf += "<br><br><i>";
    buf += tr("Revert all transactions starting with the following one:");
    buf += "</i></b></p><p>";
    buf += targetTr;
    buf += "</p><p><b>";
    buf += tr("Are you sure you want to rollback?");
    buf += "</b></p></qt>";

    mydialog = new Dialog(buf);
    mydialog->setAttribute(Qt::WA_QuitOnClose);
    mydialog->setWindowTitle(tr("Warning!"));
    mydialog->exec();

    // Do nothing, if user canceled
    if (mydialog->getStatus()) {
        ui->urpm_status->setText(tr("Action canceled by user!"));
        return;
    }

    cmd += "'" + targetTr.toStdString() + "'";
    termlog(cmd);
    if (system(cmd.c_str()) != 0) {
        list_transactions();
        termlog("An error happend during command execution.");
        ui->urpm_status->setText(tr("WARNING: Error during execution!"));
    } else {
        list_transactions();
        termlog("The system has been rolled back to " + targetTr.toStdString());
        ui->urpm_status->setText(
            tr("The system has been rolled back!"));
    }
}


/**
 * remove()
 * uninstalls the selected package via urpme
 * \note this function may only be called via prepareRemove(), to ensure the
 *       right data was added.
 */
void qt4urpm::remove()
{
	// Inform user about depending packages
	QString buf;
	if (dependencies.size()) {
		buf  = "<qt><p><b>";
		buf += tr("The following packages depend on those you selected:");
		buf += "</b></p><p>";
		buf += toUList(dependencies);
		buf += "</p><p><b>";
		buf += tr("They must be removed as well. Is it okay, to go on?");
		buf += "</b></p></qt>";

		mydialog = new Dialog(buf);
		mydialog->setAttribute(Qt::WA_QuitOnClose);
		mydialog->setWindowTitle(tr("Warning!"));
		mydialog->exec();

		// Do nothing, if user canceled
		if (mydialog->getStatus()) {
			ui->urpm_status->setText(tr("Action canceled by user!"));
			return;
		}
		selectedOrphans += dependencies;
	}

	string cmd("/usr/sbin/urpme "); // urpme --force is too mighty, better do not use it here

	// show a dialog to ask the user whether he/she really wants to remove the
	// packages "cmd"
	buf  = "<qt><p><b>";
	buf += tr("You are about to do a system wide change!");
	buf += "<br><br><i>";
	buf += tr("Selected packages:");
	buf += "</i></b></p><p>";
	buf += toUList(selectedOrphans);
	buf += "</p><p><b>";
	buf += tr("Are you sure you want to remove these packages?");
	buf += "</b></p></qt>";

	mydialog = new Dialog(buf);
	mydialog->setAttribute(Qt::WA_QuitOnClose);
	mydialog->setWindowTitle(tr("Warning!"));
	mydialog->exec();

	// Do nothing, if user canceled
	if (mydialog->getStatus()) {
		ui->urpm_status->setText(tr("Action canceled by user!"));
		return;
	}

	// Remove the '\n' qt adds
	selectedOrphans.remove('\n');

	// Check if remove would work without problems
	string check("/usr/bin/urpmq --not-available | grep '");
	QString bufstring;

	// remove the first space character
	bufstring = selectedOrphans.remove(0, 1);
	//replace the left spaces with '\|' for the grep operation
	bufstring = bufstring.replace(" ", "\\|");
	check += bufstring.toStdString();
	check += "'";
	termlog(check);
	ui->urpm_status->setText(tr("Testing removal!"));
	if (system(check.c_str()) == 0) { // if packages are not in the repositories.
		int ret = 0;
		termlog("An error happend during command execution.");
		ui->urpm_status->setText
			(tr("WARNING:") + " " + tr("Packages not available!"));
		QString msg("<qt><p><b>");
		msg += tr("WARNING:");
		msg += "</b></p><p>";
		msg +=
			tr("The selected packages do not exist in the installed "
			   "repositories. This will probably let an automatic deinstallation "
			   "fail. Do you wish to start a manual deinstallation in a terminal?");
		msg += "</p></qt>";
		ret = QMessageBox::warning
			(this, tr("Packages not available!"), msg,
			 QMessageBox::Yes | QMessageBox::No );
		if (ret == QMessageBox::Yes) {
			//Start xterm to perform a manual deinstallation
			QString terminalstring("/usr/bin/xterm -e \"urpme ");
			terminalstring += selectedOrphans;
			terminalstring += "\"";
			termlog(terminalstring.toStdString());
			if (system(terminalstring.toStdString().c_str()) != 0) {
				termlog("An error happend during command execution.");
				ui->urpm_status->setText(tr("WARNING: Error during execution!"));
				msg = "<qt><p><b>";
				msg += tr("WARNING:");
				msg += "</b></p><p>";
				msg +=
					tr("qt4urpm failed to start the deinstallation routine in a "
					   "terminal! Might it be, that xterm is not installed? Are the "
					   "urpm* tools installed at all?");
				msg += "</p></qt>";
				QMessageBox::warning
					(this, tr("Can not start the terminal program!"), msg,
					 QMessageBox::Ok);
			}
			orphansearch();
			return;
		} else {
			termlog("Action canceled\n");
			orphansearch();
			ui->urpm_status->setText(tr("Action canceled by user"));
			return;
		}
	}
	ui->urpm_status->setText(tr("Test has been passed. Remove packages ..."));
	// Finally remove packages
	cmd += selectedOrphans.toStdString();
	termlog(cmd);
	if(system(cmd.c_str()) != 0) {
		orphansearch();
		termlog("An error happend during command execution.");
		ui->urpm_status->setText(tr("WARNING: Error during execution!"));
	} else {
		orphansearch();
		termlog("Removed:" + selectedOrphans.toStdString());
		ui->urpm_status->setText(tr("Successfully removed packages!"));
	}
}

/// reformates a Qstring to show up as unsorted list (QTs richtext implemnet.)
QString qt4urpm::toUList(QString input)
{
	//QString temp(input);
	QString formString("<ul><li><nobr>");
	formString += input.replace("\n", "</nobr></li><li><nobr>");
	formString += "</nobr></li></ul>";
	return formString;
}

/// Shows the online help
void qt4urpm::showHelp()
{
	QMessageBox msgBox;
	QString buf("<qt><h3>");
	buf += tr("qt4urpm help");
	buf += "</h3><p>";
	buf += tr("qt4urpm provides an easy to use user interface to handle orphaned packages and to search "
		       "for packages providing a certain file.");
	buf += "</p><p>";
	buf += tr("To handle one or more packages that are marked as orphaned in the package database and so "
		       "are listed in the \"orphaned\" tab, select the package via left clicking it (to select more "
		       "than one package, hold down CTRL and left click all wished packages in the list). Afterwards "
		       "select the button providing the wished option (remove or mark as not orphaned).");
	buf += "</p><p>";
	buf += tr("To search for a package providing a specific file, switch to the \"search\" tab, enter the "
		       "name of the searched file in the searchbox and click the search button.\n"
		       "Once the search is finished, all packages that provide a file with the name you searched for "
		       "are listed in the table. Select the wished one via left clicking it and click \"install\".");
	buf += "</p><p>";
	buf += tr("For more information, bug tracker and feature requests please visit our project page at ");
	buf += HOMEPAGE;
	buf += "</p></qt>";
	msgBox.about(this, tr("Help"), buf);

	/*
	// Open help - this should be replaced with a local help file
	// or with a better fitting link.
	QUrl url("http://www.sf.net/projects/qt4urpm");
	if(!QDesktopServices::openUrl(url))
		termlog("Failed to open help!");
	*/
}

/// Shows the about screen
void qt4urpm::showAbout()
{
	QMessageBox msgBox;
	QString buf("<qt><h3>");
	buf += tr("This is qt4urpm version ");
	buf += QT4URPMI_VERSION;
	buf += "</h3><p>";
	buf += tr("qt4urpm is a frontend for the urpm* package management "
		       "tools, developed by users of the MandrivaUser.de Community.\n");
	buf += tr("For more information, bug tracker and feature requests please visit our project page at ");
	buf += HOMEPAGE;
	buf += "</p><p>";
	buf += COPYRIGHT;
	buf += "</p><p>";
	// Make clear what GPL means for the user: NO WARRANTY :)
	buf += "This program is distributed in the hope that it will be useful, "
	       "but WITHOUT ANY WARRANTY; without even the implied warranty of "
	       "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the "
	       "GNU General Public License for more details.";
	buf += "</p></qt>";
	msgBox.about(this, tr("About qt4urpm"), buf);
}

/// Shows the standard about QT4 screen
void qt4urpm::showAboutQt()
{
	QApplication::aboutQt();
}

/// Sets the search button as default, if text in find_lineEdit is changed
void qt4urpm::buttonAct()
{
	ui->find_search->setDefault(true);
}

void qt4urpm::changeTooltip(int r, int c)
{
	int row = r;
	int column = c;
	QString sum = "...";
	QString buf(ui->orph_results->item(row, column)->data(Qt::DisplayRole).toString());
	QString summary = "rpm -qi ";
	summary+=buf;
	summary.remove("\n");
	sumproc.setStandardOutputProcess(&grepproc);
	sumproc.start(summary);
	grepproc.start("grep Summary");
	grepproc.waitForFinished();
	sum = grepproc.readAllStandardOutput();
	sum.remove("Summary     :");
	sum.remove("\n");
	ui->orph_results->setToolTip(sum);
	sumproc.close();
	grepproc.close();
}

