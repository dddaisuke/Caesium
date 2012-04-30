#ifndef LOADPREVIEW_H
#define LOADPREVIEW_H

#include <QThread>
#include <QImage>

class LoadPreview : public QThread
{
    Q_OBJECT
public:
    explicit LoadPreview(QString fileName, QObject *parent = 0);
    void run();

signals:
    void imageLoaded1(QImage);
    void imageLoaded2(QImage);
    void clearSignal();
    void loadingCompressedPreview();

public slots:

private:
    QString t_fileName;

};

#endif // LOADPREVIEW_H
