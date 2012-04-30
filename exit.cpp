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


#include "exit.h"
#include "ui_exit.h"
#include "global.h"

Exit::Exit(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Exit)
{
    ui->setupUi(this);

    e_parent = parent;
    this->setWindowFlags(Qt::MSWindowsFixedSizeDialogHint);
    this->setWindowFlags(Qt::Dialog);

    ui->alwaysCheckBox->setChecked(settings.value("Preferences/promptexit").value<bool>());
}

Exit::~Exit()
{
    delete ui;
}


void Exit::on_alwaysCheckBox_toggled(bool checked)
{
    settings.setValue("Preferences/promptexit", checked);
}
