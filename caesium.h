#ifndef CAESIUM_H
#define CAESIUM_H

#include <QMainWindow>
#include <QLabel>
#include <QNetworkReply>

#include <compressionthread.h>
#include "preferences.h"
#include "about.h"
#include "updater.h"
#include "exit.h"

namespace Ui {
    class Caesium;
}

class Caesium : public QMainWindow
{
    Q_OBJECT

public:
    explicit Caesium(QWidget *parent = 0);
    ~Caesium();

private:
    Ui::Caesium *ui;
    Preferences *pf;
    AboutDialog *about;
    Exit *exit;
    Updater *updater;
    QLabel *imageLabel1;
    QLabel *imageLabel2;

private slots:
    void on_actionRemove_Item_From_List_and_Hard_Disk_triggered();
    void on_outputLineEdit_textChanged(QString );
    void on_zoomOutToolButton_clicked();
    void on_zoomInToolButton_clicked();
    void on_actionOpen_Folder_triggered();
    void on_actionSettings_triggered();
    void on_originalToolButton_clicked();
    void on_fitToolButton_clicked();
    void on_suffixLineEdit_textChanged(QString text);
    void on_cancelButton_clicked();
    void on_actionCompress_triggered();
    void on_updateButton_clicked();
    void on_actionShow_Toolbar_toggled(bool t_checked);
    void on_actionCheck_for_Updates_triggered();
    void on_actionAbout_Qt_triggered();
    void on_actionOpen_List_triggered();
    void on_actionSave_List_As_triggered();
    void on_previewButton_clicked();
    void on_listTreeWidget_itemSelectionChanged();
    void on_actionPreview_triggered();
    void on_setQualityButton_clicked();
    void on_cantSetButton_clicked();
    void on_formatComboBox_currentIndexChanged(int);
    void on_undoButton_clicked();
    void on_sameFolderCheckBox_clicked(bool checked);
    void on_browseToolButton_clicked();
    void on_actionDonate_to_Caesium_triggered();
    void on_actionOnline_Support_triggered();
    void on_actionWebsite_triggered();
    void on_actionClear_List_triggered();
    void on_removeButton_clicked();
    void on_actionRemove_Item_triggered();
    void on_addButton_clicked();
    void on_actionAbout_triggered();
    void on_actionExit_triggered();
    void closeEvent(QCloseEvent *event);
    void on_actionAdd_Pictures_triggered();
    void showVersionLabel(bool);
    void checkForUpdateStartup();
    QStringList loadImages();
    QStringList loadQuality();
    void showMessageBox();
    void updateUI(int, int, QString);
    void processingIcon(int, QString);
    void fitWindow();
    void originalSize();
    void scaleImage(double factor);
    bool duplicateCheck(QString name, QString dir);
    void adjustScrollbars(int v, int h, int p);
    void enableButtons(bool flag);
    void countUpdate();
    void afterPFUpdate();
    int outputFolderCreation();
    void openList();
    void lstSave();
    void on_actionSave_List_triggered();
    void toolbarIconSize();
    void on_action32x32_triggered();
    void on_action24x24_triggered();
    void on_actionSave_Profile_triggered();
    void on_actionLoad_Profile_triggered();
    void profileLoadEngine(QString fileName);
    void on_compressButton_clicked();
    void setImageToLabel1(QImage image);
    void setImageToLabel2(QImage image);
    void adjustPreviewView();
    void loadingCompressedPreview();

signals:
    void loadThreadStop();
};

#endif // CAESIUM_H
