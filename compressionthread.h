#ifndef COMPRESSIONTHREAD_H
#define COMPRESSIONTHREAD_H

#include <QtGui>

class CompressionThread : public QThread
{
    Q_OBJECT

public:
    CompressionThread(QStringList list, QString dir, QString format, QString suffix, QStringList quality, bool checked, QObject *parent);
    void run();
    void noEnlarge();
    void writeToFile(QString orig, QString dest);
    void keepDate(QString, QString);

private:
    QStringList t_list;
    QString t_dir;
    QString t_format;
    QString t_suffix;
    QStringList t_quality;
    bool t_checked;

signals:
    void updateUI(int value, int newSize, QString ratio);
    void processingIcon(int, QString);
};

#endif // COMPRESSIONTHREAD_H
