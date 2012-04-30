#ifndef STARTUPUPDATER_H
#define STARTUPUPDATER_H

#include <QtGui>
#include <QNetworkReply>
#include <QNetworkAccessManager>

class StartupUpdater : public QThread
{
    Q_OBJECT

public:
    StartupUpdater(QObject *parent);
    void run();

private slots:
    void downloadFinished(QNetworkReply*);

private:

signals:
    void showLabel(bool flag);

};

#endif // STARTUPUPDATER_H
