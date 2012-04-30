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

#include <QtGui/QApplication>
#include <QString>
#include "caesium.h"
#include "global.h"
#include <QDebug>
#include <QTextStream>

QString findLocaleN(int n, QString dir)
{
    QDir langDir(dir + "/language");
    QStringList list = langDir.entryList(QStringList("*.qm"), QDir::Files, QDir::Name);
    for (int i = list.length() - 1; i >= 0; i--)
    {
        QString file = list.at(i);
        QStringList name = file.split(".");
        if (name[0] == QString::number(n))
        {
            return QString::number(n) + "." + name[1];
        }
    }
    return QString("31.English");
}

QString findLocaleS(QString string, QString dir)
{
    QDir langDir(dir + "/language");
    QStringList list = langDir.entryList(QStringList("*.qm"), QDir::Files, QDir::Name);
    for (int i = list.length() - 1; i >= 0; i--)
    {
        QString file = list.at(i);
        QStringList name = file.split(".");
        if (name[1] == string)
        {
            return name[0] + "." + string;
        }
    }
    return QString("31.English");
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    settings.setIniCodec("UTF-8");

    a.addLibraryPath(a.applicationDirPath() + "/lib/");

    int loc = QLocale::system().language();
    QString locale = settings.value("Preferences/lang").value<QString>();
    QTranslator translator;
    if(locale.isEmpty())
    {
        translator.load(a.applicationDirPath() + "/language/" + findLocaleN(loc, a.applicationDirPath()));
        a.installTranslator(&translator);
    }
    else
    {
        translator.load(a.applicationDirPath() + "/language/" + findLocaleS(locale, a.applicationDirPath()));
        a.installTranslator(&translator);
    }

    QPixmap pixmap(":icons/splash.png");
    QSplashScreen splash(pixmap, Qt::WindowStaysOnTopHint);
    splash.setMask(pixmap.mask());
    splash.show();


    Caesium w;
    QTimer::singleShot(400, &splash, SLOT(close()));
    QTimer::singleShot(400, &w, SLOT(show()));

    return a.exec();
}
