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

#include "about.h"
#include "ui_about.h"
#include "global.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);

    ui->infoTextEdit->setHtml("<strong>" +
                              tr("Name:") + "</strong> Caesium<br /><strong>" +
                              tr("Version:") + "</strong> " + version_string + "<br /><strong>" +
                              tr("Author:") + "</strong> Matteo Paonessa<br /><strong>eMail:</strong> matteo.paonessa@gmail.com<br /><strong>" +
                              tr("Website:") + "</strong> http://caesium.sourceforge.net");

    ui->thanksTextEdit->setHtml("<p><strong>" + tr("Translations:") + "</strong></p>" +
                                "<ul><li>FR - DODICH</li><li>JP - Tilt</li><li>DE - Markus</li><li>TW - Zkm</li> <li>SV - Åke Engelbrektson</li> <li>CN - Rob</li></ul>" +
                                "<p><strong>Pyexiv2</strong> - http://tilloy.net/dev/pyexiv2/<br />" +
                                tr("For Metatag manipulation") + "<br /><br /><strong>Tango Desktop Project </strong>- http://tango.freedesktop.org/<br />" +
                                "<strong>Elementary Icons </strong>- http://danrabbit.deviantart.com/art/elementary-Icons-65437279<br />" +
                                tr("Base of the icon set"));


}

AboutDialog::~AboutDialog()
{
    delete ui;
}

void AboutDialog::on_closeButton_clicked()
{
    this->close();
}
