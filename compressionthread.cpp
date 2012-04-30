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

#include "compressionthread.h"
#include "caesium.h"
#include "ui_caesium.h"
#include "global.h"

#include <QSettings>
#include <QFile>

#include <iostream>


//WCHAR *qStringToWideChar(const QString &str)
//{
//    if (str.isNull())
//        return 0;
//    WCHAR *result = new WCHAR[str.length() + 1];
//    for (int i = 0; i < str.length(); ++i)
//        result[i] = str[i].unicode();
//    result[str.length()] = 0;
//    return result;
//}

//void startProcess(QString argv)
//{
//    WCHAR* exe = qStringToWideChar(argv);
//    STARTUPINFO si;
//    PROCESS_INFORMATION pi;

//    ZeroMemory( &si, sizeof(si) );
//    si.cb = sizeof(si);
//    ZeroMemory( &pi, sizeof(pi) );

//    if( !CreateProcessW( NULL,
//                         exe,
//                        NULL,
//                        NULL,
//                        FALSE,
//                        CREATE_NO_WINDOW,
//                        NULL,
//                        NULL,
//                        &si,
//                        &pi )
//        )
//        {
//        return;
//    }

//    WaitForSingleObject( pi.hProcess, INFINITE );

//    CloseHandle( pi.hProcess );
//    CloseHandle( pi.hThread );
//}

CompressionThread::CompressionThread(QStringList list, QString dir, QString format, QString suffix, QStringList quality, bool checked, QObject *parent)
: QThread(parent)
{
    t_list = list;
    t_dir = dir;
    t_format = format;
    t_suffix = suffix;
    t_quality = quality;
    t_checked = checked;
    settings.setIniCodec("UTF-8");
}

QString t_getRatio(int origSize, int newSize)
{
    int ratio = newSize * 100 / origSize;
    if (ratio <= 100)
    {
        return QString::number(ratio - 100) + " %";
    }
    else
    {
        return "+" + QString::number(ratio - 100) + " %";
    }
}

void CompressionThread::run()
{
    QString outputBaseFolder = t_dir;
    bool no_enlarge = settings.value("Preferences/noenlarge").value<bool>();
    if (no_enlarge)
    {
        CompressionThread::noEnlarge();
        return;
    }
    for (int i = 0; i < t_list.count(); i++)
    {
        QFileInfo info(t_list.at(i));
        if (info.size() == 0)
        {
            emit updateUI(i + 1, 0, "0%");
            continue;
        }
        if (QFile::exists(t_list.at(i)))
        {
            int old_size = info.size();
            emit processingIcon(i, info.fileName());
            QImage image(t_list.at(i));
            if (t_checked)
            {
                t_dir = info.path();
                image.save(t_dir + "/" + info.completeBaseName() + t_suffix, t_format.toLatin1(), t_quality.at(i).toInt());
            }
            else
            {
                image.save(t_dir + "/" + info.completeBaseName() + t_suffix, t_format.toLatin1(), t_quality.at(i).toInt());
            }


            if (settings.value("Preferences/exif").value<bool>() && t_format.toLower() == "jpg")
            {
                QString exec = "tools\\exif_copy.exe \"" + t_list.at(i) + "\" \"" + t_dir + "\\" + info.completeBaseName() + t_suffix +"\"";
                //startProcess(exec);
            }
            QFile newImage(t_dir + "/" + info.completeBaseName() + t_suffix);
            QFileInfo newInfo;
            if (QFile::exists(t_dir + "/" + info.completeBaseName() + t_suffix + "." + t_format))
            {
                QFile::remove(t_dir + "/" + info.completeBaseName() + t_suffix + "." + t_format);
            }
            newImage.rename(t_dir + "/" + info.completeBaseName() + t_suffix, t_dir + "/" + info.completeBaseName() + t_suffix + "." + t_format);
            newInfo.setFile(t_dir + "/" + info.completeBaseName() + t_suffix + "." + t_format);
            int size = newInfo.size();
            QString ratio = t_getRatio(old_size, newInfo.size());
            if(settings.value("Preferences/keepdate").value<bool>())
            {
                CompressionThread::keepDate(t_list.at(i), newInfo.filePath());
            }

            emit updateUI(i + 1, size, ratio);
        }
        else
        {
            emit updateUI(i + 1, -1, "0%");
        }
    }
    exit();
}

void CompressionThread::noEnlarge()
{
    for (int i = 0; i < t_list.count(); i++)
    {
        QFileInfo info(t_list.at(i));
        if (info.size() == 0)
        {
            emit updateUI(i + 1, 0, "0%");
            continue;
        }
        if (QFile::exists(t_list.at(i)))
        {
            int old_size = info.size();
            emit processingIcon(i, info.fileName());
            QImage image(t_list.at(i));
            if (t_checked)
            {
                t_dir = info.path();
                image.save(t_dir + "/" + info.completeBaseName() + t_suffix + "." + t_format + ".ckd", t_format.toLatin1(), t_quality.at(i).toInt());
            }
            else
            {
                image.save(t_dir + "/" + info.completeBaseName() + t_suffix + "." + t_format + ".ckd", t_format.toLatin1(), t_quality.at(i).toInt());
            }

            if (settings.value("Preferences/exif").value<bool>() && t_format.toLower() == "jpg")
            {
                QString exec = "tools\\exif_copy.exe \"" + t_list.at(i) + "\" \"" + t_dir + "\\" + info.completeBaseName() + t_suffix + "." + t_format + "\"";
                //startProcess(exec);
            }

            QFileInfo newInfo(t_dir + "/" + info.completeBaseName() + t_suffix + "." + t_format + ".ckd");
            int size = newInfo.size();
            QString ratio = t_getRatio(old_size, newInfo.size());

            if (newInfo.size() > info.size())
            {
                size *= -1;
                QFile::remove(t_dir + "/" + info.completeBaseName() + t_suffix + "." + t_format + ".ckd");
            }
            else
            {
                QFile::remove(t_dir + "/" + info.completeBaseName() + t_suffix + "." + t_format);
                QFile::rename(t_dir + "/" + info.completeBaseName() + t_suffix + "." + t_format + ".ckd", t_dir + "/" + info.completeBaseName() + t_suffix + "." + t_format);
            }

            if(settings.value("Preferences/keepdate").value<bool>())
            {
                CompressionThread::keepDate(t_list.at(i), t_dir + "/" + info.completeBaseName() + t_suffix + "." + t_format);
            }

            emit updateUI(i + 1, size, ratio);
        }
        else if (info.size() == 0)
        {
            emit updateUI(i + 1, 0, "0%");
        }
        else
        {
            emit updateUI(i + 1, -1, "0%");
        }
    }
}

void CompressionThread::keepDate(QString orig, QString dest)
{
//    HANDLE hFile, hFile2;

//    FILETIME ftCreate, ftAccess, ftWrite;
//    hFile = CreateFile(qStringToWideChar(orig), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
//    GetFileTime(hFile, &ftCreate, &ftAccess, &ftWrite);

//    hFile2 = CreateFile(qStringToWideChar(dest), FILE_WRITE_ATTRIBUTES, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
//    SetFileTime(hFile2, &ftCreate, &ftAccess, &ftWrite);

//    CloseHandle(hFile);
//    CloseHandle(hFile2);
}
