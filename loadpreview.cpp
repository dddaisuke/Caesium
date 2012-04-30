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

#include "loadpreview.h"

#include <QFileInfo>
#include <QDir>
#include <QDebug>

LoadPreview::LoadPreview(QString fileName, QObject *parent) :
    QThread(parent)
{
    t_fileName = fileName;
}

void LoadPreview::run()
{
    emit clearSignal();
    QString tempDir = QDir::tempPath();
    QImage image(t_fileName);
    QFileInfo tempFile(tempDir + "/_caesium_/" + QFileInfo(t_fileName).fileName() + ".cae");
    if (tempFile.exists())
    {
        emit loadingCompressedPreview();
        QImage image2(tempDir + "/_caesium_/" + QFileInfo(t_fileName).fileName() + ".cae");
        emit imageLoaded2(image2);
    }
    else
    {
        emit clearSignal();
    }
    emit imageLoaded1(image);
    this->exit();
}
