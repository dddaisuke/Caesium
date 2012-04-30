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

#include "startupupdater.h"
#include "caesium.h"
#include "ui_caesium.h"
#include "global.h"

#include <QNetworkReply>

StartupUpdater::StartupUpdater(QObject *parent): QThread(parent)
{ }

void StartupUpdater::run()
{
    QNetworkAccessManager *m_NetworkMngr = new QNetworkAccessManager();
    QString url = "http://caesium.sourceforge.net/current_version.txt";
    connect(m_NetworkMngr, SIGNAL(finished(QNetworkReply*)), this, SLOT(downloadFinished(QNetworkReply*)));
    m_NetworkMngr->get(QNetworkRequest(url));
    this->exec();
}

void StartupUpdater::downloadFinished(QNetworkReply* reply)
{
    if(reply->error() == QNetworkReply::NoError)
    {
        QString version = reply->readAll();
        if (version_build < version.toInt())
        {
            emit showLabel(true);
        }
        else
        {
            emit showLabel(false);
        }
    }
    else
    {
        emit showLabel(false);
    }
    reply->close();
    this->exit(0);
}
