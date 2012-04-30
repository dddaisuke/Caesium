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

#include "updater.h"
#include "ui_updater.h"
#include "global.h"
#include <QtCore>
#include <QtGui>
#include <QtNetwork>

#include <stdlib.h>
#include <stdio.h>

QString c_v = "";
QString fileName;
int d = 0, p = 0;
bool downFinished = false;

Updater::Updater(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Updater)
{
    ui->setupUi(this);

    u_parent = parent;

    ui->progressBar->setValue(1);
    ui->progressBar->setRange(0,0);

    initGetCurrentVersion();
}

Updater::~Updater()
{
    delete ui;
}

QString buildToString(QString build)
{
    build.insert(1, ".");
    build.insert(3, ".");
    return build;
}

QString toRealURL(QString u)
{
    QStringList splitted;
    splitted = u.split('=');
    return "http://" + splitted[1] + ".dl.sourceforge.net/project/caesium/" + c_v + "/" + fileName;
}

void Updater::initGetCurrentVersion()
{
    QString url = "http://caesium.sourceforge.net/current_version.txt";
    QNetworkAccessManager *m_NetworkMngr = new QNetworkAccessManager(this);
    m_NetworkMngr->get(QNetworkRequest(url));
    connect(m_NetworkMngr, SIGNAL(finished(QNetworkReply*)), this, SLOT(getCurrentVersion(QNetworkReply*)));
}

void Updater::getCurrentVersion(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError)
    {
        QString reply_result = reply->readAll();
        int c_v_int = reply_result.toInt();
        c_v = buildToString(reply_result);
        reply->close();
        ui->statusLabel->setText(tr("Caesium current version is: ") + "<b>" + c_v + "</b>");
        ui->progressBar->setRange(0,1);
        if (c_v_int > version_build)
        {
            ui->startButton->setEnabled(true);
        }
        ui->versionLabel->setText(ui->versionLabel->text() + "<b> " + version_string + "</b>");
        fileName = "caesium-" + c_v + "-win.exe";
        getChangelog();
    }
    else if (reply->error() == QNetworkReply::TimeoutError)
    {
        ui->statusLabel->setText(tr("ERROR: Connection timed out..."));
        ui->progressBar->setRange(0,1);
    }
    else
    {
        ui->statusLabel->setText(tr("An error occurred. Please check your internet connection."));
        ui->progressBar->setRange(0,1);
    }
    ui->progressBar->setValue(0);
}

void Updater::on_startButton_clicked()
{
    downFinished = false;
    ui->startButton->setEnabled(false);
    QString url = "http://downloads.sourceforge.net/project/caesium/" + c_v + "/" + fileName;
    ui->statusLabel->setText(tr("Contacting server..."));
    QNetworkAccessManager *m_NetworkMngr = new QNetworkAccessManager(this);
    m_NetworkMngr->get(QNetworkRequest(url));
    connect(m_NetworkMngr, SIGNAL(finished(QNetworkReply*)), this, SLOT(getUrl(QNetworkReply*)));
}
void Updater::getUrl(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError)
    {
        QString url = toRealURL(reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString());
        initDownload(url);
    }
    else if (reply->error() == QNetworkReply::TimeoutError)
    {
        ui->statusLabel->setText(tr("ERROR: Connection timed out..."));
        ui->startButton->setEnabled(true);
    }
    else
    {
        ui->statusLabel->setText(tr("An error occurred. Please check your internet connection."));
        ui->startButton->setEnabled(true);
    }
}

void Updater::initDownload(QString url)
{
    ui->statusLabel->setText(tr("Downloading ") + fileName + "...");
    QNetworkAccessManager *m_NetworkMngr = new QNetworkAccessManager(this);
    QNetworkReply *reply= m_NetworkMngr->get(QNetworkRequest(url));
    QEventLoop loop;
    connect(reply, SIGNAL(finished()),&loop, SLOT(quit()));
    connect(reply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(updateProgressBar(qint64, qint64)));
    loop.exec();

    if (reply->error() == QNetworkReply::NoError)
    {
        file.setFileName(QDir::tempPath() + "/" + fileName);
        file.open(QIODevice::WriteOnly);
        file.write(reply->readAll());
        file.close();
        ui->statusLabel->setText(tr("Download completed."));
        int r = QMessageBox::information(this, tr("Information"), tr("Caesium will exit now in order to allow the update to run"), QMessageBox::Ok);
        if (r == QMessageBox::Ok)
        {
            downFinished = true;
            reply->close();
            delete reply;
            this->close();
        }
    }
    else if (reply->error() == QNetworkReply::TimeoutError)
    {
        ui->statusLabel->setText(tr("ERROR: Connection timed out..."));
    }
    else
    {
        ui->statusLabel->setText(tr("An error occurred. Please check your internet connection."));
    }
    ui->startButton->setEnabled(true);

}

void Updater::updateProgressBar(qint64 done, qint64 total)
{
    d = done;
    ui->progressBar->setMaximum(total);
    ui->progressBar->setValue(done);
}

void Updater::closeEvent(QCloseEvent *event)
{
    if (downFinished)
    {
        QProcess *process;
        process->startDetached(QDir::tempPath() + "/" + fileName);
        u_parent->close();
    }
    event->setAccepted(true);
}

void Updater::on_closeButton_clicked()
{
    this->close();
}

void Updater::getChangelog()
{
    QString url = "http://caesium.sourceforge.net/changelog.txt";
    QNetworkAccessManager *networkMngr = new QNetworkAccessManager(this);
    networkMngr->get(QNetworkRequest(url));
    connect(networkMngr, SIGNAL(finished(QNetworkReply*)), this, SLOT(getChangelogHTML(QNetworkReply*)));
}

void Updater::getChangelogHTML(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError)
    {
        QString reply_result = reply->readAll();
        ui->changelogTextEdit->setText(reply_result);
    }
    else
    {
        ui->changelogTextEdit->setText("Failed to retrieve the changelog");
    }
    reply->close();
}
