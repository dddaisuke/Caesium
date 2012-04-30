/*******************************************************************************
#
# Copyright (C) 2010-2011 Matteo Paonessa <matteo.paonessa@gmail.com>
#
# This file is part of the Caesium distribution.
#
# Caesium is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# Caesium is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Caesium; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, 5th Floor, Boston, MA 02110-1301 USA.
#
# Author: Matteo Paonessa <matteo.paonessa@gmail.com>
#
# ******************************************************************************/

#include "preferences.h"
#include "ui_preferences.h"
#include "global.h"

#include <QSettings>
#include <QFileDialog>
#include <QDir>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QMessageBox>

#include <QtDebug>


Preferences::Preferences(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Preferences)
{
    ui->setupUi(this);
    settings.setIniCodec("UTF-8");
    loadSettings();
    loadLanguages();
    ui->menuListWidget->item(0)->setSelected(true);
}

Preferences::~Preferences()
{
    delete ui;
}

void Preferences::loadLanguages()
{
    QDir langDir(QDir::currentPath() + "/language");
    QStringList list = langDir.entryList(QStringList("*.qm"), QDir::Files, QDir::NoSort);
    for (int i = list.length() - 1; i >= 0; i--)
    {
        QString file = list.at(i);
        QStringList infoList;
        QStringList name = file.split(".");
        infoList << name[1];
        QTreeWidgetItem *item = new QTreeWidgetItem(infoList, 0);
        item->setIcon(0, QIcon(":/icons/" + name[1] + ".png"));
        ui->languageTreeWidget->insertTopLevelItem(0, item);
    }
    ui->languageTreeWidget->sortItems(0, Qt::AscendingOrder);
}

void Preferences::loadSettings()
{
    ui->useDefaultPathCheckBox->setChecked(settings.value("Preferences/defaultpath").value<bool>());
    ui->defaultOutputLineEdit->setText(settings.value("Settings/path").value<QString>());
    if (settings.value("Preferences/imageview").value<QString>() == "fit")
    {
        ui->fitWindowRadio->setChecked(true);
        ui->originalSizeRadio->setChecked(false);
    }
    else
    {
        ui->fitWindowRadio->setChecked(false);
        ui->originalSizeRadio->setChecked(true);
    }
    ui->checkUpdatesCheckBox->setChecked(settings.value("Preferences/startupupdt").value<bool>());
    ui->keepExifCheckBox->setChecked(settings.value("Preferences/exif").value<bool>());
    ui->doNotEnlargeCheckBox->setChecked(settings.value("Preferences/noenlarge").value<bool>());
    ui->deleteCheckBox->setChecked(settings.value("Preferences/delete").value<bool>());
    ui->keepDateCheckBox->setChecked(settings.value("Preferences/keepdate").value<bool>());
    ui->scanSubDirCheckBox->setChecked(settings.value("Preferences/scansubdir").value<bool>());
    ui->promptBeforeExitCheckBox->setChecked(settings.value("Preferences/promptexit").value<bool>());
    ui->loadLastProfileCheckBox->setChecked(settings.value("Preferences/loadprofile").value<bool>());
}


void Preferences::on_browseToolButton_clicked()
{
    QString outputPath = QFileDialog::getExistingDirectory(this, tr("Select default output directory"),
                                            "/home",
                                            QFileDialog::ShowDirsOnly
                                            | QFileDialog::DontResolveSymlinks);
    if (outputPath != "")
    {
        ui->defaultOutputLineEdit->setText(outputPath);
    }
}


void Preferences::on_OKButton_clicked()
{
    QDir dir(ui->defaultOutputLineEdit->text());
    if (ui->defaultOutputLineEdit->text() == "")
    {
        ui->useDefaultPathCheckBox->setChecked(false);
    }
    else if (!dir.exists() && ui->defaultOutputLineEdit->isEnabled() == true)
    {
        dir.mkpath(ui->defaultOutputLineEdit->text());
    }
    settings.setValue("Preferences/defaultpath", ui->useDefaultPathCheckBox->isChecked());
    settings.setValue("Settings/path", ui->defaultOutputLineEdit->text());
    if (ui->originalSizeRadio->isChecked())
    {
        settings.setValue("Preferences/imageview", "orig");
    }
    else
    {
        settings.setValue("Preferences/imageview", "fit");
    }
    settings.setValue("Preferences/startupupdt", ui->checkUpdatesCheckBox->isChecked());
    settings.setValue("Preferences/exif", ui->keepExifCheckBox->isChecked());
    settings.setValue("Preferences/noenlarge", ui->doNotEnlargeCheckBox->isChecked());
    settings.setValue("Preferences/delete", ui->deleteCheckBox->isChecked());
    settings.setValue("Preferences/keepdate", ui->keepDateCheckBox->isChecked());
    settings.setValue("Preferences/scansubdir", ui->scanSubDirCheckBox->isChecked());
    settings.setValue("Preferences/promptexit", ui->promptBeforeExitCheckBox->isChecked());
    settings.setValue("Preferences/loadprofile", ui->loadLastProfileCheckBox->isChecked());
    if (ui->languageTreeWidget->selectedItems().count() != 0)
    {
        settings.setValue("Preferences/lang", ui->languageTreeWidget->selectedItems().at(0)->text(0));
        QMessageBox::information(this,
                                 tr("Information"),
                                 tr("You need to restart the application before\nchanges take effect"),
                                 QMessageBox::Ok);
    }

    this->close();
}

void Preferences::on_cancelButton_clicked()
{
    this->close();
}
