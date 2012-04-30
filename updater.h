#ifndef UPDATER_H
#define UPDATER_H

#include <QDialog>
#include <QHttp>
#include <QCloseEvent>
#include <QFile>
#include <QTimer>
#include <QtNetwork>

namespace Ui {
    class Updater;
}

class Updater : public QDialog
{
    Q_OBJECT

public:
    explicit Updater(QWidget *parent = 0);
    ~Updater();

private:
    Ui::Updater *ui;
    QFile file;
    QWidget *u_parent;

private slots:
    void on_closeButton_clicked();
    void on_startButton_clicked();
    void initGetCurrentVersion();
    void getCurrentVersion(QNetworkReply*);
    void getUrl(QNetworkReply*);
    void initDownload(QString);
    void updateProgressBar(qint64, qint64);
    void closeEvent(QCloseEvent *event);
    void getChangelog();
    void getChangelogHTML(QNetworkReply*);
};

#endif // UPDATER_H
