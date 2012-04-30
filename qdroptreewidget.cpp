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

#include "qdroptreewidget.h"
#include "global.h"

#include <QtGui>
#include <QStringList>

QDropTreeWidget::QDropTreeWidget(QWidget *parent)
    : QTreeWidget(parent)
{
    createActions();
    createMenus();
}

bool QDropTreeWidget::d_duplicateCheck(QString name, QString dir)
{
    for (int i = 0; i < this->topLevelItemCount(); i++)
    {
        if (name == this->topLevelItem(i)->text(0) && dir == this->topLevelItem(i)->text(5))
        {
            return false;
        }
    }
    return true;
}

bool checkExtension(QString extension)
{
    if (extension.toLower() == "jpg" || extension.toLower() == "png" || extension.toLower() == "bmp" || extension.toLower() == "jpeg" || extension.toLower() == "tif" || extension.toLower() == "tiff" || extension.toLower() == "ppm" || extension.toLower() == "xbm" || extension.toLower() == "xpm")
    {
        return true;
    }
    return false;
}

QString d_fixedSize(int size)
{
    double sizeD = (double) size;
    if (sizeD < 1024 && sizeD > -1)
    {
        return QString::number(sizeD, 'f', 2) + " Bytes";
    }
    else
    {
        return QString::number(sizeD/1024, 'f', 2) + " Kb";
    }
}

void QDropTreeWidget::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}

//void QDropTreeWidget::dragMoveEvent(QDragMoveEvent *event)
//{

//}

void QDropTreeWidget::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    QList<QUrl> urlList = mimeData->urls();
    if (mimeData->hasFormat("text/uri-list"))
    {
        for (int i = urlList.size() - 1; i >= 0 ; i--)
        {
            QString path = urlList.at(i).path();
            path = path.right(path.length() - 1);
            QFileInfo *fileInfo = new QFileInfo(path);
            QStringList infoList;
            if (checkExtension(fileInfo->suffix()) && QDropTreeWidget::d_duplicateCheck(fileInfo->fileName(), fileInfo->absoluteFilePath()))
            {
                infoList << fileInfo->fileName() << d_fixedSize(fileInfo->size()) << "" << "" << "" << fileInfo->absoluteFilePath();
                QTreeWidgetItem *item = new QTreeWidgetItem(infoList, 0);
                item->setIcon(0, QIcon(":/icons/added.png"));
                this->insertTopLevelItem(0, item);
            }
        }
    }
    event->acceptProposedAction();
    emit countUpdate();
}

void QDropTreeWidget::contextMenuEvent(QContextMenuEvent *event)
{
    QTreeWidgetItem* item = itemAt(event->pos());
    if (item)
    {
        originalAction->setEnabled(checkInput());
        destinationAction->setEnabled(checkDestination());
        itemMenu->exec(event->globalPos());
    }
    else
    {
        clearAction->setEnabled(this->topLevelItemCount() > 0);
        noItemMenu->exec(event->globalPos());
    }
}

void QDropTreeWidget::createActions()
{
    clearAction = new QAction(QIcon(":/icons/clear.png"), tr("Clear list"), this);
    connect(clearAction, SIGNAL(triggered()), this, SLOT(actionClear()));

    previewAction = new QAction(QIcon(":/icons/preview.png"), tr("Preview"), this);
    connect(previewAction, SIGNAL(triggered()), this, SLOT(actionPreview()));

    removeAction = new QAction(QIcon(":/icons/remove.png"), tr("Remove item"), this);
    connect(removeAction, SIGNAL(triggered()), this, SLOT(actionRemove()));

    addAction = new QAction(QIcon(":/icons/add.png"), tr("Add pictures..."), this);
    connect(addAction, SIGNAL(triggered()), this, SLOT(actionAdd()));

    addFolderAction = new QAction(QIcon(":/icons/open_folder.png"), tr("Open folder..."), this);
    connect(addFolderAction, SIGNAL(triggered()), this, SLOT(actionAddFolder()));

    destinationAction = new QAction(QIcon(":/icons/output_dir.png"), tr("Open destination folder"), this);
    connect(destinationAction, SIGNAL(triggered()), this, SLOT(openDestination()));

    originalAction = new QAction(QIcon(":/icons/input_dir.png"), tr("Open input folder"), this);
    connect(originalAction, SIGNAL(triggered()), this, SLOT(openInput()));

    removeFromHDDAction = new QAction(QIcon(":/icons/remove_hdd.png"), tr("Remove from list and Hard Disk"), this);
    connect(removeFromHDDAction, SIGNAL(triggered()), this, SLOT(actionRemoveFromHDD()));
}

void QDropTreeWidget::createMenus()
{
    noItemMenu = new QMenu(this);
    noItemMenu->addAction(addAction);
    noItemMenu->addAction(addFolderAction);
    noItemMenu->addSeparator();
    noItemMenu->addAction(clearAction);

    itemMenu = new QMenu(this);
    itemMenu->addAction(addAction);
    itemMenu->addAction(addFolderAction);
    itemMenu->addSeparator();
    itemMenu->addAction(removeAction);
    itemMenu->addAction(removeFromHDDAction);
    itemMenu->addAction(clearAction);
    itemMenu->addSeparator();
    itemMenu->addAction(previewAction);
    itemMenu->addSeparator();
    itemMenu->addAction(originalAction);
    itemMenu->addAction(destinationAction);
}

void QDropTreeWidget::actionPreview()
{
    emit preview();
}

void QDropTreeWidget::actionAdd()
{
    emit add();
}

void QDropTreeWidget::actionAddFolder()
{
    emit addFolder();
}

void QDropTreeWidget::actionRemove()
{
    emit remove();
}

void QDropTreeWidget::actionRemoveFromHDD()
{
    emit removeHDD();
}

void QDropTreeWidget::actionClear()
{
    emit a_clear();
}

void QDropTreeWidget::openDestination()
{
    if (!same_folder_flag)
    {
        QDesktopServices::openUrl(QUrl("file:///" + out_folder));
    }
    else
    {
        QDropTreeWidget::openInput();
    }
}

void QDropTreeWidget::openInput()
{
    QFileInfo info(this->selectedItems().at(0)->text(5));
    QDesktopServices::openUrl(QUrl("file:///" + info.absolutePath()));
}

bool QDropTreeWidget::checkDestination()
{
    if (same_folder_flag)
    {
        return QDropTreeWidget::checkInput();
    }
    else if (out_folder == "")
    {
        return false;
    }
    else
    {
        return true;
    }
}

bool QDropTreeWidget::checkInput()
{
    QFileInfo first;
    QFileInfo second;
    for (int i = 0; i < this->selectedItems().count() - 1; i++)
    {
        first.setFile(this->selectedItems().at(i)->text(5));
        second.setFile(this->selectedItems().at(i + 1)->text(5));
        if (first.absolutePath().operator !=(second.absolutePath()))
        {
            return false;
        }
    }
    return true;
}
