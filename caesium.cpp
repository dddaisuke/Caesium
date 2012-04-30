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

#include "caesium.h"
#include "ui_caesium.h"
#include "preferences.h"
#include "about.h"
#include "startupupdater.h"
#include "global.h"
#include "exit.h"
#include "loadpreview.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QCloseEvent>
#include <QUrl>
#include <QDesktopServices>
#include <QFile>
#include <QTextStream>
#include <QSettings>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

QTime t;
int error_count = 0, iteration_count = 0, skipped_count;
double scale_factor = 1.0;
bool preview_on = false;
bool orig_on = true;
bool cancelled = false;
QString list_file, previous_list, last_profile = settings.value("Settings/profile").value<QString>();
int item_count, saved_space;
QImage currentImage;

//WCHAR *m_qStringToWideChar(const QString &str)
//{
//    if (str.isNull())
//        return 0;
//    WCHAR *result = new WCHAR[str.length() + 1];
//    for (int i = 0; i < str.length(); ++i)
//        result[i] = str[i].unicode();
//    result[str.length()] = 0;
//    return result;
//}

//void m_startProcess(QString argv)
//{
//    WCHAR* exe = m_qStringToWideChar(argv);
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
//                        CREATE_NO_WINDOW ,
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

void removeDir(QString path)
{
    QDir dir(path);
    if (dir.count() == 0)
    {
        dir.rmdir(path);
    }
    else
    {
        QStringList entries = dir.entryList();
        for (int i = 0; i < entries.count(); i++)
        {
            dir.remove(entries[i]);
        }
        dir.rmdir(path);
    }
}

QString loadLastDir()
{
    QFile file("lst");
    if (file.open(QIODevice::ReadOnly))
    {
        QTextStream stream(&file);
        return stream.readLine(0);
    }
    else
    {
        return "";
    }
}

QString fixedSize(int size, int flag)
{
    double sizeD = (double) size;
    if (flag == 1 && sizeD > 1024*1024)
    {
        return QString::number(sizeD/1024/1024, 'f', 2) + " Mb";
    }
    if (sizeD < 1024 && sizeD > -1)
    {
        return QString::number(sizeD, 'f', 2) + " Bytes";
    }
    else
    {
        return QString::number(sizeD/1024, 'f', 2) + " Kb";
    }
}

QString getRatio(int origSize, int newSize)
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

QStringList findFilesRecursively(QString directory_path, QStringList filters)
{
    QStringList result;
    QDirIterator directory_walker(directory_path, filters, QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);
    directory_path.replace('\\', '/');
    directory_path.append('/');
    QString full_path;
    while(directory_walker.hasNext())
    {
        directory_walker.next();
        full_path = directory_walker.fileInfo().filePath();

        result << full_path.replace(directory_path, "", Qt::CaseSensitive);
    }
    return result;
}

Caesium::Caesium(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Caesium)
{
    ui->setupUi(this);

    QDir::setCurrent(QFileInfo(QCoreApplication::arguments().at(0)).absolutePath());

    settings.setIniCodec("UTF-8");

    ui->scrollArea1->verticalScrollBar()->setSliderDown(false);
    ui->scrollArea2->verticalScrollBar()->setSliderDown(false);

    connect(ui->scrollArea1->verticalScrollBar(), SIGNAL(valueChanged(int)), ui->scrollArea2->verticalScrollBar(), SLOT(setValue(int)));
    connect(ui->scrollArea2->verticalScrollBar(), SIGNAL(valueChanged(int)), ui->scrollArea1->verticalScrollBar(), SLOT(setValue(int)));
    connect(ui->scrollArea1->horizontalScrollBar(), SIGNAL(valueChanged(int)), ui->scrollArea2->horizontalScrollBar(), SLOT(setValue(int)));
    connect(ui->scrollArea2->horizontalScrollBar(), SIGNAL(valueChanged(int)), ui->scrollArea1->horizontalScrollBar(), SLOT(setValue(int)));

    connect(ui->listTreeWidget, SIGNAL(countUpdate()), this, SLOT(countUpdate()));
    connect(ui->listTreeWidget, SIGNAL(preview()), this, SLOT(on_actionPreview_triggered()));
    connect(ui->listTreeWidget, SIGNAL(a_clear()), this, SLOT(on_actionClear_List_triggered()));
    connect(ui->listTreeWidget, SIGNAL(remove()), this, SLOT(on_actionRemove_Item_triggered()));
    connect(ui->listTreeWidget, SIGNAL(removeHDD()), this, SLOT(on_actionRemove_Item_From_List_and_Hard_Disk_triggered()));
    connect(ui->listTreeWidget, SIGNAL(add()), this, SLOT(on_actionAdd_Pictures_triggered()));
    connect(ui->listTreeWidget, SIGNAL(addFolder()), this, SLOT(on_actionOpen_Folder_triggered()));

    ui->mainToolBar->setVisible(settings.value("Preferences/showtoolbar").value<bool>());
    ui->actionShow_Toolbar->setChecked(settings.value("Preferences/showtoolbar").value<bool>());

    QActionGroup *actionGroup = new QActionGroup(this);
    actionGroup->addAction(ui->action24x24);
    actionGroup->addAction(ui->action32x32);
    Caesium::toolbarIconSize();

    ui->updateButton->setVisible(false);

    ui->suffixErrorLabel->setVisible(false);

    ui->cantSetButton->setVisible(ui->formatComboBox->currentIndex() != 0);

    QList<int> size;
    size << 1 << 145;

    ui->listTreeWidget->setColumnWidth(0, 170);
    ui->listTreeWidget->setColumnWidth(1, 70);
    ui->listTreeWidget->setColumnWidth(2, 70);
    ui->listTreeWidget->setColumnWidth(3, 70);
    ui->listTreeWidget->setColumnWidth(4, 65);
    ui->listTreeWidget->setColumnWidth(5, 100);

    ui->splitter->setSizes(size);

    imageLabel1 = new QLabel;
    imageLabel2 = new QLabel;

    ui->scrollArea1->setWidget(imageLabel1);
    imageLabel1->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    imageLabel1->setScaledContents(true);

    ui->scrollArea2->setWidget(imageLabel2);
    imageLabel2->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    imageLabel2->setScaledContents(true);
    ui->scrollArea2->setWindowTitle("TITOLO");

    QStringList lastDirArray = loadLastDir().split("|");
    if (lastDirArray.at(0) == "1")
    {
        ui->outputLineEdit->setText(lastDirArray.at(1));
        ui->rememberDirectoryCheckBox->setChecked(true);
    }

    if (settings.value("Preferences/loadprofile").value<bool>())
    {
        Caesium::profileLoadEngine(last_profile);
    }

    ui->deleteOriginalPushButton->setVisible(settings.value("Preferences/delete").value<bool>());

    Caesium::checkForUpdateStartup();

    out_folder = ui->outputLineEdit->text();

    countUpdate();

    if (QCoreApplication::arguments().count() > 1)
    {
        Caesium::profileLoadEngine(QApplication::arguments().at(1));
    }
    Caesium::openList();

    srand(time(NULL));


}

Caesium::~Caesium()
{
    delete ui;
}

void Caesium::adjustScrollbars(int v, int h, int p)
{
    ui->scrollArea1->verticalScrollBar()->setValue(v + p);
    ui->scrollArea1->horizontalScrollBar()->setValue(h + p);
}

bool Caesium::duplicateCheck(QString name, QString dir)
{
    for (int i = 0; i < ui->listTreeWidget->topLevelItemCount(); i++)
    {
        if (name == ui->listTreeWidget->topLevelItem(i)->text(0) && dir == ui->listTreeWidget->topLevelItem(i)->text(5))
        {
            return false;
        }
    }
    return true;
}

QStringList Caesium::loadImages()
{
    QStringList imageList;
    for (int i = 0; i < ui->listTreeWidget->topLevelItemCount(); i++)
    {
        imageList += ui->listTreeWidget->topLevelItem(i)->text(5);
    }
    return imageList;
}

QStringList Caesium::loadQuality()
{
    QStringList qualityList;
    for (int i = 0; i < ui->listTreeWidget->topLevelItemCount(); i++)
    {
        if (ui->listTreeWidget->topLevelItem(i)->text(4) != "")
        {
            qualityList += ui->listTreeWidget->topLevelItem(i)->text(4);
        }
        else
        {
            qualityList += QString::number(ui->qualitySlider->value());
        }
    }
    return qualityList;
}

void Caesium::closeEvent(QCloseEvent *event)
{
    if (settings.value("Preferences/promptexit").value<bool>() == true)
    {
        exit = new Exit(this);
        if (exit->exec() == 1)
        {
            lstSave();
            settings.setValue("Settings/profile", last_profile);
            event->setAccepted(true);
        }
        else
        {
            event->setAccepted(false);
        }
    }
    else
    {
        lstSave();
        settings.setValue("Settings/profile", last_profile);
        event->setAccepted(true);
    }
}

void Caesium::on_actionAdd_Pictures_triggered()
{
    QStringList filePaths;
    QString dir = (settings.value("Settings/dir").value<QString>());
    QString path;

    if (dir == "")
    {
        dir = QDesktopServices::storageLocation(QDesktopServices::PicturesLocation);
    }

    filePaths = QFileDialog::getOpenFileNames(
            this, tr("Select file(s)"), dir, tr("Supported Images (*.bmp *.jpg *.jpeg *.tif *.tiff *.png *.ppm *.xbm *.xpm);;PNG Files (*.png);;JPEG Files (*.jpg *.jpeg);;BMP Files (*.bmp);;TIFF Files (*.tif *.tiff);;PPM Files (*.ppm);;XBM Files (*.xbm);;XPM Files (*.xpm)"));

    if (filePaths.size() != 0)
    {
        ui->progressBar->setMaximum(filePaths.size());
    }

    for (int i = filePaths.size() - 1; i > 0 - 1; i--)
    {
        QFileInfo *fileInfo = new QFileInfo(filePaths.at(i));
        QStringList infoList;
        if (Caesium::duplicateCheck(fileInfo->fileName(), fileInfo->absoluteFilePath()))
        {
            infoList << fileInfo->fileName() << fixedSize(fileInfo->size(), 0) << "" << "" << "" << fileInfo->absoluteFilePath();
            QTreeWidgetItem *item = new QTreeWidgetItem(infoList, 0);
            item->setIcon(0, QIcon(":/icons/added.png"));

            ui->listTreeWidget->insertTopLevelItem(0, item);
            ui->progressBar->setValue(filePaths.size() - i);
            path = fileInfo->path();
        }
    }

    if (path != "")
    {
        settings.setValue("Settings/dir", path);
    }

    countUpdate();
    ui->progressBar->setValue(0);
}

void Caesium::on_actionOpen_Folder_triggered()
{
    QString dir = (settings.value("Settings/dir").value<QString>());

    if (dir == "")
    {
        dir = QDesktopServices::storageLocation(QDesktopServices::PicturesLocation);
    }
    QString directory = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                 dir,
                                                 QFileDialog::ShowDirsOnly
                                                 | QFileDialog::DontResolveSymlinks);

    QStringList filters, fileList;
    filters << "*.bmp" << "*.jpg" << "*.jpeg" << "*.tif" << "*.png" << "*.ppm" << "*.xbm" << "*.xpm";
    if(settings.value("Preferences/scansubdir").value<bool>())
    {
        fileList = findFilesRecursively(directory, filters);
    }
    else
    {
        QDir qDir(directory);
        qDir.setNameFilters(filters);
        fileList = qDir.entryList(filters);
    }
    if (fileList.size() != 0)
    {
        ui->progressBar->setMaximum(fileList.size());
    }

    for (int i = fileList.size() - 1; i > 0 - 1; i--)
    {
        QFileInfo *fileInfo = new QFileInfo(directory + "/" + fileList.at(i));
        QStringList infoList;
        if (Caesium::duplicateCheck(fileInfo->fileName(), fileInfo->absoluteFilePath()))
        {
            infoList << fileInfo->fileName() << fixedSize(fileInfo->size(), 0) << "" << "" << "" << fileInfo->absoluteFilePath();
            QTreeWidgetItem *item = new QTreeWidgetItem(infoList, 0);
            item->setIcon(0, QIcon(":/icons/added.png"));
            ui->listTreeWidget->insertTopLevelItem(0, item);
            ui->progressBar->setValue(fileList.size() - i);
        }
    }

    if (directory != "")
    {
        settings.setValue("Settings/dir", directory);
    }

    countUpdate();
    ui->progressBar->setValue(0);
}

void Caesium::on_actionExit_triggered()
{
    this->close();
}

void Caesium::on_actionAbout_triggered()
{
    about = new AboutDialog(this);
    about->show();
}

void Caesium::on_addButton_clicked()
{
    Caesium::on_actionAdd_Pictures_triggered();
}

void Caesium::on_actionRemove_Item_triggered()
{
    int index = 0;
    for (int i = 0; i < ui->listTreeWidget->topLevelItemCount(); i++)
    {
        if (ui->listTreeWidget->topLevelItem(i)->isSelected())
        {
            ui->listTreeWidget->takeTopLevelItem(i);
            i--;
            index = i;
        }
    }
    if (ui->listTreeWidget->topLevelItemCount() != 0 && index > 0)
    {
        ui->listTreeWidget->topLevelItem(index)->setSelected(true);
    }
    else
    {
        imageLabel1->setPixmap(QPixmap(""));
        imageLabel2->setPixmap(QPixmap(""));
    }
    countUpdate();
    imageLabel1->adjustSize();
    imageLabel2->adjustSize();
}

void Caesium::on_removeButton_clicked()
{
    Caesium::on_actionRemove_Item_triggered();
}

void Caesium::on_actionClear_List_triggered()
{
    ui->listTreeWidget->clear();
    imageLabel1->setPixmap(QPixmap(""));
    imageLabel2->setPixmap(QPixmap(""));
    countUpdate();
    imageLabel1->adjustSize();
    imageLabel2->adjustSize();
}

void Caesium::on_actionWebsite_triggered()
{
    QDesktopServices::openUrl(QUrl("http://caesium.sourceforge.net"));
}

void Caesium::on_actionOnline_Support_triggered()
{
    QDesktopServices::openUrl(QUrl("http://caesium.sourceforge.net/support.html"));
}

void Caesium::on_actionDonate_to_Caesium_triggered()
{
    QDesktopServices::openUrl(QUrl("http://sourceforge.net/donate/index.php?group_id=336104"));
}

void Caesium::on_browseToolButton_clicked()
{
    QString outputPath = QFileDialog::getExistingDirectory(this, tr("Select output directory"),
                                            "/home",
                                            QFileDialog::ShowDirsOnly
                                            | QFileDialog::DontResolveSymlinks);
    if (outputPath != "")
    {
        ui->outputLineEdit->setText(outputPath);
    }
}

void Caesium::on_sameFolderCheckBox_clicked(bool checked)
{
    if (checked && ui->suffixLineEdit->text() == "")
    {
        ui->suffixLineEdit->setText("_compressed");
    }
    same_folder_flag = checked;
}

void Caesium::on_undoButton_clicked()
{
    ui->suffixLineEdit->setText("");
}

void Caesium::on_formatComboBox_currentIndexChanged(int index)
{
    ui->qualityLabel->setEnabled(index == 0);
    ui->qualitySlider->setEnabled(index == 0);
    ui->qualitySpinBox->setEnabled(index == 0);
    ui->sameLevelCheckBox->setEnabled(index == 0);
    ui->setQualityButton->setEnabled(index == 0);
    ui->cantSetButton->setVisible(index != 0);
}

void Caesium::on_cantSetButton_clicked()
{
    QMessageBox::information(this, tr("Info"),
                             tr("PNG and BMP are loseless formats, and\ncan't be compressed like JPG do.\nCaesium will set the quality level automatically\nfor those formats."), QMessageBox::Ok);
}

void Caesium::on_setQualityButton_clicked()
{
    if (ui->sameLevelCheckBox->isChecked())
    {
        for (int i = 0; i < ui->listTreeWidget->topLevelItemCount(); i++)
        {
            ui->listTreeWidget->topLevelItem(i)->setText(4, QString::number(ui->qualitySlider->value()));
        }
    }
    else if (ui->listTreeWidget->selectedItems().count() != 0)
    {
        for (int i = 0; i < ui->listTreeWidget->selectedItems().count(); i++)
        {
            ui->listTreeWidget->selectedItems().at(i)->setText(4, QString::number(ui->qualitySlider->value()));
        }
    }
}

void Caesium::on_actionPreview_triggered()
{

    QString tempFolder = QDir::tempPath();
    QDir caesiumTemp(tempFolder + "/" + "_caesium_");
    if (!caesiumTemp.exists())
    {
        caesiumTemp.mkpath(tempFolder + "/" + "_caesium_");
    }
    if (ui->listTreeWidget->selectedItems().count() != 0)
    {
        for (int i = 0; i < ui->listTreeWidget->selectedItems().count(); i++)
        {
            QString rndString = ui->listTreeWidget->selectedItems().at(i)->text(0);
            QFileInfo info(tempFolder + "/_caesium_/" + rndString + ".cae");
            QFileInfo orig(ui->listTreeWidget->selectedItems().at(i)->text(5));
            QImage image(ui->listTreeWidget->selectedItems().at(i)->text(5));
            if (ui->formatComboBox->currentIndex() == 0)
            {
                image.save(tempFolder + "/_caesium_/" + rndString + ".cae", ui->formatComboBox->currentText().toLower().toAscii(), ui->qualitySlider->value());
            }
            else
            {
                image.save(tempFolder + "/_caesium_/" + rndString + ".cae", ui->formatComboBox->currentText().toLower().toAscii(), 1);
            }
            if (settings.value("Preferences/exif").value<bool>())
            {
                QString exec = "tools\\exif_copy.exe \"" + ui->listTreeWidget->selectedItems().at(i)->text(5) + "\" \"" + tempFolder + "/_caesium_/" + rndString + ".cae\"";
                //m_startProcess(exec);
            }
            ui->listTreeWidget->selectedItems().at(i)->setText(2, fixedSize(info.size(), 0));
            ui->listTreeWidget->selectedItems().at(i)->setText(3, getRatio(orig.size(), info.size()));
            ui->listTreeWidget->selectedItems().at(i)->setText(4, QString::number(ui->qualitySlider->value()));
            if (i == ui->listTreeWidget->selectedItems().count() - 1)
            {
                QImage image2(tempFolder + "/_caesium_/" + rndString + ".cae");
                imageLabel2->setPixmap(QPixmap::fromImage(image2));
                imageLabel2->resize(image2.size());
            }
        }
        preview_on = true;
        if(settings.value("Preferences/imageview").value<QString>() == "fit" || !orig_on)
        {
            Caesium::fitWindow();
        }
        else
        {
            Caesium::scaleImage(1.0);
        }
        Caesium::adjustScrollbars(ui->scrollArea1->verticalScrollBar()->value(), ui->scrollArea1->horizontalScrollBar()->value(), 1);
        Caesium::adjustScrollbars(ui->scrollArea1->verticalScrollBar()->value(), ui->scrollArea1->horizontalScrollBar()->value(), -1);
    }
}

void Caesium::on_previewButton_clicked()
{
    Caesium::on_actionPreview_triggered();
}

void Caesium::on_listTreeWidget_itemSelectionChanged()
{
    emit loadThreadStop();
    scale_factor = 1.0;
    if (ui->listTreeWidget->selectedItems().count() != 0 && ui->listTreeWidget->selectedItems().count() < 2)
    {
        QMovie *movie = new QMovie(":/icons/loading.gif");
        imageLabel1->setMovie(movie);
        movie->start();
        imageLabel1->setGeometry(0, 0, 32, 32);
        QThread *loadThread = new LoadPreview(ui->listTreeWidget->selectedItems().at(0)->text(5), this);
        connect(loadThread, SIGNAL(imageLoaded1(QImage)), this, SLOT(setImageToLabel1(QImage)));
        connect(loadThread, SIGNAL(imageLoaded2(QImage)), this, SLOT(setImageToLabel2(QImage)));
        connect(loadThread, SIGNAL(clearSignal()), imageLabel2, SLOT(clear()));
        connect(loadThread, SIGNAL(loadingCompressedPreview()), this, SLOT(loadingCompressedPreview()));
        connect(this, SIGNAL(loadThreadStop()), loadThread, SLOT(terminate()));
        connect(loadThread, SIGNAL(finished()), this, SLOT(adjustPreviewView()));
        loadThread->start();
        preview_on = false;
        Caesium::enableButtons(true);
    }
    else if (ui->listTreeWidget->selectedItems().count() == 0)
    {
        Caesium::enableButtons(false);
    }
}

void Caesium::on_actionSave_List_As_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save as..."), QString::null, tr("Caesium List (*.clf)"));
    QFile file (fileName);
    if (file.open(QIODevice::WriteOnly))
    {
        QTextStream stream(&file);
        for (int i = 0; i < ui->listTreeWidget->topLevelItemCount(); i++)
        {
            stream << ui->listTreeWidget->topLevelItem(i)->text(0) + "|"
                    << ui->listTreeWidget->topLevelItem(i)->text(1) + "|"
                    << ui->listTreeWidget->topLevelItem(i)->text(2) + "|"
                    << ui->listTreeWidget->topLevelItem(i)->text(3) + "|"
                    << ui->listTreeWidget->topLevelItem(i)->text(4) + "|"
                    << ui->listTreeWidget->topLevelItem(i)->text(5)
                    << endl;
        }
    }
    opened_list = fileName;
}

void Caesium::on_actionOpen_List_triggered()
{
    QString listName = QFileDialog::getOpenFileName(this, tr("Open a list file..."), QString::null, tr("Caesium List (*.clf)"));
    QFile list(listName);
    if (list.open(QIODevice::ReadOnly))
    {
        ui->listTreeWidget->clear();
        QTextStream stream(&list);
        QString line = stream.readLine(0);
        while (line != QString::null)
        {
            QStringList splittedLine = line.split("|");
            QTreeWidgetItem *item = new QTreeWidgetItem(splittedLine, 0);
            item->setIcon(0, QIcon(":/icons/added.png"));
            ui->listTreeWidget->insertTopLevelItem(0, item);
            line = stream.readLine(0);
        }
    }
    opened_list = listName;
    Caesium::countUpdate();
}

void Caesium::on_actionAbout_Qt_triggered()
{
    QMessageBox::aboutQt(this, tr("About Qt4"));
}

void Caesium::on_actionCheck_for_Updates_triggered()
{
    ui->actionCheck_for_Updates->setEnabled(false);

    updater = new Updater(this);
    updater->show();

    ui->actionCheck_for_Updates->setEnabled(true);
}


void Caesium::checkForUpdateStartup()
{
    if (!settings.value("Preferences/startupupdt").value<bool>())
    {
        return;
    }
    ui->actionCheck_for_Updates->setEnabled(false);
    QThread *updtThread = new StartupUpdater(this);
    connect(updtThread, SIGNAL(showLabel(bool)), this, SLOT(showVersionLabel(bool)));
    updtThread->start();
}

void Caesium::showVersionLabel(bool flag)
{
    ui->actionCheck_for_Updates->setEnabled(true);
    if (flag)
    {
        ui->updateButton->setVisible(true);
    }
    else
    {
        ui->updateButton->setVisible(false);
    }
}

void Caesium::on_actionShow_Toolbar_toggled(bool t_checked)
{
    ui->mainToolBar->setVisible(t_checked);
    settings.setValue("Preferences/showtoolbar", t_checked);
}

void Caesium::on_updateButton_clicked()
{
    Caesium::on_actionCheck_for_Updates_triggered();
}

void Caesium::on_actionCompress_triggered()
{
    cancelled = false;
    error_count = 0;
    iteration_count = 0;
    skipped_count = 0;
    t.start();
    QString outputDir = ui->outputLineEdit->text();
    QString format = ui->formatComboBox->currentText().toLower();
    QString suffix = ui->suffixLineEdit->text();
    bool checked = ui->sameFolderCheckBox->isChecked();

    if ((outputDir != "" || checked == true) || (settings.value("Preferences/defaultpath").value<bool>() && outputDir == ""))
    {
        if (ui->listTreeWidget->topLevelItemCount() != 0)
        {
            if (outputDir == "")
            {
                outputDir = settings.value("Settings/path").value<QString>();
            }
            else if (!QDir(outputDir).exists())
            {
                int r = Caesium::outputFolderCreation();
                if (r == 0)
                {
                    if (!QDir().mkdir(outputDir))
                    {
                        QMessageBox::critical(this, tr("Error"),
                                              tr("Failed to create the directory.\nAborting."),
                                              QMessageBox::Ok);
                        return;
                    }
                }
                else
                {
                    return;
                }
            }
            ui->progressBar->setMaximum(ui->listTreeWidget->topLevelItemCount());
            QStringList imageList = loadImages();
            QStringList qualityList = loadQuality();

            QThread *compThread = new CompressionThread(imageList, outputDir, format, suffix, qualityList, checked, this);
            connect(compThread, SIGNAL(updateUI(int, int, QString)), this, SLOT(updateUI(int, int, QString)));
            connect(compThread, SIGNAL(processingIcon(int, QString)), this, SLOT(processingIcon(int, QString)));
            connect(compThread, SIGNAL(finished()), this, SLOT(showMessageBox()));
            connect(ui->cancelButton, SIGNAL(clicked()), compThread, SLOT(terminate()));
            compThread->start();
            ui->actionCompress->setEnabled(false);
            ui->compressButton->setEnabled(false);
            ui->cancelButton->setEnabled(true);
        }
        else
        {
            QMessageBox::critical(this, tr("Error"), tr("The list is empty! Fill it!"), QMessageBox::Ok);
        }
    }
    else
    {
        QMessageBox::critical(this, tr("Error"), tr("Set an Output Directory first!"), QMessageBox::Ok);
    }
}

void Caesium::showMessageBox()
{
    ui->cancelButton->setEnabled(false);
    ui->actionCompress->setEnabled(true);
    ui->compressButton->setEnabled(true);
    double time = t.elapsed();
    if(settings.value("Preferences/delete").value<bool>())
    {
        ui->progressLabel->setText(tr("Deleting original files..."));
        ui->progressBar->setMaximum(ui->listTreeWidget->topLevelItemCount());
        for (int i = 0; i < ui->listTreeWidget->topLevelItemCount(); i++)
        {
            if (QFile::exists(ui->listTreeWidget->topLevelItem(i)->text(5)))
            {
                QFile::remove(ui->listTreeWidget->topLevelItem(i)->text(5));
                ui->progressBar->setValue(i);
            }
            else
            {

            }
        }
    }
    countUpdate();
    if (!cancelled)
    {
        QMessageBox::information(this, tr("Compression done!"), tr("Compression Completed with:\n» ")
                                 + QString::number(ui->listTreeWidget->topLevelItemCount() - skipped_count - error_count) + tr(" compressed\n» ")
                                 + QString::number(skipped_count) + tr(" skipped\n» ")
                                 + QString::number(error_count) + tr(" error(s)\nTime elapsed: ")
                                 + QString::number(time / 1000, 'f', 2) + " s."
                                 + tr("\nSaved space: ") + fixedSize(saved_space, 1), QMessageBox::Ok);
    }
    saved_space = 0;
}

void Caesium::updateUI(int i, int newSize, QString ratio)
{
    if (newSize < 0)
    {
        ui->listTreeWidget->topLevelItem(i - 1)->setIcon(0, QIcon(":/icons/skipped.png"));
        skipped_count++;
        newSize = 0;
    }
    else if (newSize == 0)
    {
        ui->listTreeWidget->topLevelItem(i - 1)->setIcon(0, QIcon(":/icons/error.png"));
        error_count++;
    }
    else
    {
        ui->listTreeWidget->topLevelItem(i - 1)->setIcon(0, QIcon(":/icons/done.png"));
        saved_space += QFileInfo(ui->listTreeWidget->topLevelItem(i - 1)->text(5)).size() - newSize;
    }
    ui->progressBar->setValue(i);
    ui->listTreeWidget->topLevelItem(i - 1)->setText(2, fixedSize(newSize, 0));
    ui->listTreeWidget->topLevelItem(i - 1)->setText(3, ratio);
}

void Caesium::processingIcon(int i, QString name)
{
    iteration_count = i;
    ui->listTreeWidget->scrollToItem(ui->listTreeWidget->topLevelItem(i - 1), QAbstractItemView::EnsureVisible);
    ui->progressLabel->setText("(" + QString::number(i + 1) + "/" + QString::number(ui->listTreeWidget->topLevelItemCount()) + ") " +
                               + "Processing: " + name);
    ui->listTreeWidget->topLevelItem(i)->setIcon(0, QIcon(":/icons/processing.png"));
}

void Caesium::on_cancelButton_clicked()
{
    ui->actionCompress->setEnabled(true);
    ui->compressButton->setEnabled(true);
    ui->listTreeWidget->topLevelItem(iteration_count)->setIcon(0, QIcon(":/icons/error.png"));
    ui->progressBar->setValue(0);
    cancelled = true;
}

void Caesium::on_suffixLineEdit_textChanged(QString text)
{
    QRegExp rx("[\\/|:*<>]");
    if (text.contains(rx))
    {
        ui->suffixErrorLabel->setVisible(true);
        ui->actionCompress->setEnabled(false);
        ui->compressButton->setEnabled(false);
    }
    else if (!text.contains(rx) && ui->listTreeWidget->topLevelItemCount() != 0)
    {
        ui->suffixErrorLabel->setVisible(false);
        ui->actionCompress->setEnabled(true);
        ui->compressButton->setEnabled(true);

    }
    else
    {
        ui->suffixErrorLabel->setVisible(false);
        ui->actionCompress->setEnabled(false);
        ui->compressButton->setEnabled(false);
    }
}

void Caesium::fitWindow()
{
    ui->zoomInToolButton->setEnabled(false);
    ui->zoomOutToolButton->setEnabled(false);
    scale_factor = 1.0;

    imageLabel1->resize(ui->scrollArea1->size());
    ui->scrollArea1->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->scrollArea1->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->scrollArea2->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->scrollArea2->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    double ratio = (double)currentImage.width() / (double)currentImage.height();
    double s_ratio = (double) ui->scrollArea1->width() / (double) ui->scrollArea1->height();
    if ((ratio >= 1.0 && ratio >= s_ratio) || (ratio < 1.0 && ratio >= s_ratio))
    {
        imageLabel1->resize(ui->scrollArea1->width(), ui->scrollArea1->width() / ratio);
        imageLabel2->resize(ui->scrollArea1->width(), ui->scrollArea1->width() / ratio);
    }
    else
    {
        imageLabel1->resize(ui->scrollArea1->height() * ratio, ui->scrollArea1->height());
        imageLabel2->resize(ui->scrollArea1->height() * ratio, ui->scrollArea1->height());
    }

    orig_on = false;
}

void Caesium::originalSize()
{
    scale_factor = 1.0;
    ui->scrollArea1->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->scrollArea1->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->scrollArea2->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->scrollArea2->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->zoomInToolButton->setEnabled(true);
    ui->zoomOutToolButton->setEnabled(true);
    imageLabel1->adjustSize();
    imageLabel2->adjustSize();

    orig_on = true;

}

void Caesium::on_fitToolButton_clicked()
{
    Caesium::fitWindow();
}

void Caesium::on_originalToolButton_clicked()
{
    Caesium::originalSize();
}

void Caesium::on_actionSettings_triggered()
{
    pf = new Preferences(this);
    pf->show();
    connect(pf, SIGNAL(finished(int)), this, SLOT(afterPFUpdate()));
}

void Caesium::scaleImage(double factor)
{
    scale_factor *= factor;
    imageLabel1->resize(scale_factor * imageLabel1->pixmap()->size());
    if (preview_on)
    {
        imageLabel2->resize(scale_factor * imageLabel2->pixmap()->size());
    }
}

void Caesium::on_zoomInToolButton_clicked()
{
    Caesium::scaleImage(1.25);
    ui->zoomInToolButton->setEnabled(scale_factor < 2.0);
    ui->zoomOutToolButton->setEnabled(scale_factor > 0.2);
}

void Caesium::on_zoomOutToolButton_clicked()
{
    Caesium::scaleImage(0.8);
    ui->zoomInToolButton->setEnabled(scale_factor < 2.0);
    ui->zoomOutToolButton->setEnabled(scale_factor > 0.2);
}

void Caesium::enableButtons(bool flag)
{
    ui->zoomInToolButton->setEnabled(flag);
    ui->zoomOutToolButton->setEnabled(flag);
    ui->fitToolButton->setEnabled(flag);
    ui->originalToolButton->setEnabled(flag);
}

void Caesium::countUpdate()
{
    ui->progressLabel->setText(tr("Item count: ") + QString::number(ui->listTreeWidget->topLevelItemCount()));
    ui->actionClear_List->setEnabled(ui->listTreeWidget->topLevelItemCount() > 0);
    ui->actionCompress->setEnabled(ui->listTreeWidget->topLevelItemCount() > 0);
    ui->compressButton->setEnabled(ui->listTreeWidget->topLevelItemCount() > 0);
    ui->actionPreview->setEnabled(ui->listTreeWidget->topLevelItemCount() > 0);
    ui->actionRemove_Item->setEnabled(ui->listTreeWidget->topLevelItemCount() > 0);
    ui->actionRemove_Item_From_List_and_Hard_Disk->setEnabled(ui->listTreeWidget->topLevelItemCount() > 0);
    ui->removeButton->setEnabled(ui->listTreeWidget->topLevelItemCount() > 0);
    ui->previewButton->setEnabled(ui->listTreeWidget->topLevelItemCount() > 0);
    ui->actionSave_List->setEnabled((ui->listTreeWidget->topLevelItemCount() != item_count || QString::compare(opened_list, previous_list) != 0) && opened_list != "");
    item_count = ui->listTreeWidget->topLevelItemCount();
    if (opened_list != "")
    {
        this->setWindowTitle(QFileInfo(opened_list).baseName() + tr(" - Caesium - Image Compressor"));
    }
    previous_list = opened_list;
}

void Caesium::afterPFUpdate()
{
    ui->deleteOriginalPushButton->setVisible(settings.value("Preferences/delete").value<bool>());
}

void Caesium::on_outputLineEdit_textChanged(QString folder)
{
    out_folder = folder;
}

void Caesium::on_actionRemove_Item_From_List_and_Hard_Disk_triggered()
{
    QMessageBox *msg = new QMessageBox(this);
    msg->setIcon(QMessageBox::Warning);
    msg->setWindowTitle(tr("Warning"));
    msg->setText(tr("This action will delete the selected files PERMANENTLY.\nAre you sure you want to continue?"));
    msg->addButton(tr("Yes"), QMessageBox::YesRole);
    msg->addButton(tr("No"), QMessageBox::NoRole);
    int r = msg->exec();
    if (r == 0)
    {
        QFile file;
        for (int i = 0; i < ui->listTreeWidget->selectedItems().count(); i++)
        {
            file.setFileName(ui->listTreeWidget->selectedItems().at(i)->text(5));
            file.remove();
        }
        Caesium::on_removeButton_clicked();
    }
    else
    {
        return;
    }
}

int Caesium::outputFolderCreation()
{
    QMessageBox *msg = new QMessageBox(this);
    msg->setIcon(QMessageBox::Warning);
    msg->setWindowTitle(tr("Warning"));
    msg->setText(tr("The output folder doesn't exists.\nDo you want to create it?"));
    msg->addButton(tr("Yes"), QMessageBox::YesRole);
    msg->addButton(tr("Cancel"), QMessageBox::NoRole);
    return msg->exec();
}

void Caesium::openList()
{
    if (QCoreApplication::arguments().count() <= 1 || QFileInfo(QCoreApplication::arguments().at(1)).suffix() != "clf")
    {
        return;
    }
    QString listName = QCoreApplication::arguments().at(1);
    QFile list(listName);
    if (list.open(QIODevice::ReadOnly))
    {
        ui->listTreeWidget->clear();
        QTextStream stream(&list);
        QString line = stream.readLine(0);
        while (line != QString::null)
        {
            QStringList splittedLine = line.split("|");
            QTreeWidgetItem *item = new QTreeWidgetItem(splittedLine, 0);
            item->setIcon(0, QIcon(":/icons/added.png"));
            ui->listTreeWidget->insertTopLevelItem(0, item);
            line = stream.readLine(0);
        }
    }
    Caesium::countUpdate();
    opened_list = listName;
}

void Caesium::lstSave()
{
    QFile file("lst");
    if (file.open(QIODevice::WriteOnly))
    {
        QTextStream stream(&file);
        if (ui->rememberDirectoryCheckBox->isChecked())
        {
            stream << "1|" << ui->outputLineEdit->text() << endl;
        }
        else
        {
            stream << "0|" << ui->outputLineEdit->text() << endl;
        }
    }
    QString tempFolder = QDir::tempPath();
    QDir caesiumTemp(tempFolder + "/" + "_caesium_");
    if (caesiumTemp.exists())
    {
        removeDir(tempFolder + "/" + "_caesium_");
    }
}

void Caesium::on_actionSave_List_triggered()
{
    if (opened_list == "")
    {
        Caesium::on_actionSave_List_As_triggered();
        ui->actionSave_List->setEnabled(false);
        return;
    }
    QFile file (opened_list);
    if (file.open(QIODevice::WriteOnly))
    {
        QTextStream stream(&file);
        for (int i = 0; i < ui->listTreeWidget->topLevelItemCount(); i++)
        {
            stream << ui->listTreeWidget->topLevelItem(i)->text(0) + "|"
                    << ui->listTreeWidget->topLevelItem(i)->text(1) + "|"
                    << ui->listTreeWidget->topLevelItem(i)->text(2) + "|"
                    << ui->listTreeWidget->topLevelItem(i)->text(3) + "|"
                    << ui->listTreeWidget->topLevelItem(i)->text(4) + "|"
                    << ui->listTreeWidget->topLevelItem(i)->text(5)
                    << endl;
        }
    }
    ui->actionSave_List->setEnabled(false);
}

void Caesium::toolbarIconSize()
{
    QString size = settings.value("Settings/iconsize").value<QString>();
    ui->mainToolBar->setIconSize(QSize(size.split('x').at(0).toInt(), (size.split('x').at(0).toInt())));
    int int_size = size.split('x').at(0).toInt();
    switch (int_size)
    {
        case 24: ui->action24x24->setChecked(true);
        break;
        case 32: ui->action32x32->setChecked(true);
        break;
        default: ui->action32x32->setChecked(true);
    }
}

void Caesium::on_action32x32_triggered()
{
    ui->action32x32->setChecked(true);
    settings.setValue("Settings/iconsize", ui->action32x32->text());
    ui->mainToolBar->setIconSize(QSize(ui->action32x32->text().split('x').at(0).toInt(), ui->action32x32->text().split('x').at(0).toInt()));
}

void Caesium::on_action24x24_triggered()
{
    ui->action24x24->setChecked(true);
    settings.setValue("Settings/iconsize", ui->action24x24->text());
    ui->mainToolBar->setIconSize(QSize(ui->action24x24->text().split('x').at(0).toInt(), ui->action24x24->text().split('x').at(0).toInt()));
}

void Caesium::on_actionSave_Profile_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save as..."), QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation), tr("Caesium Profile (*.cpf)"));
    if (fileName != "")
    {
        QSettings profile(fileName, QSettings::IniFormat);
        profile.setIniCodec("UTF-8");
        profile.setValue("Profile/quality", ui->qualitySlider->value());
        profile.setValue("Profile/same_quality", ui->sameLevelCheckBox->isChecked());
        profile.setValue("Profile/format", ui->formatComboBox->currentIndex());
        profile.setValue("Profile/output", ui->outputLineEdit->text());
        profile.setValue("Profile/same_folder", ui->sameFolderCheckBox->isChecked());
        profile.setValue("Profile/suffix", ui->suffixLineEdit->text());
    }
    last_profile = fileName;
}

void Caesium::on_actionLoad_Profile_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select a profile file..."), QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation), tr("Caesium Profile (*.cpf)"));
    if (fileName != "")
    {
        Caesium::profileLoadEngine(fileName);
    }
}

void Caesium::profileLoadEngine(QString fileName)
{
    QSettings profile(fileName, QSettings::IniFormat);
    profile.setIniCodec("UTF-8");
    ui->qualitySlider->setValue(profile.value("Profile/quality").value<int>());
    ui->sameLevelCheckBox->setChecked(profile.value("Profile/same_quality").value<bool>());
    ui->formatComboBox->setCurrentIndex(profile.value("Profile/format").value<int>());
    ui->outputLineEdit->setText(profile.value("Profile/output").value<QString>());
    ui->sameFolderCheckBox->setChecked(profile.value("Profile/same_folder").value<bool>());
    ui->suffixLineEdit->setText(profile.value("Profile/suffix").value<QString>());
    last_profile = fileName;
}

void Caesium::on_compressButton_clicked()
{
    Caesium::on_actionCompress_triggered();
}

void Caesium::setImageToLabel1(QImage image)
{
    currentImage = image;
    imageLabel1->setPixmap(QPixmap::fromImage(image));
}

void Caesium::setImageToLabel2(QImage image)
{
    imageLabel2->setPixmap(QPixmap::fromImage(image));
}

void Caesium::adjustPreviewView()
{
    if(settings.value("Preferences/imageview").value<QString>() == "fit")
    {
        Caesium::fitWindow();
        orig_on = false;
    }
    else
    {
        Caesium::originalSize();
        orig_on = true;
    }
}

void Caesium::loadingCompressedPreview()
{
    QMovie *movie = new QMovie(":/icons/loading.gif");
    imageLabel2->setMovie(movie);
    movie->start();
    imageLabel2->setGeometry(0, 0, 32, 32);
}
