#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <QDialog>

namespace Ui {
    class Preferences;
}

class Preferences : public QDialog
{
    Q_OBJECT

public:
    explicit Preferences(QWidget *parent = 0);
    ~Preferences();

private:
    Ui::Preferences *ui;

private slots:
    void on_cancelButton_clicked();
    void on_OKButton_clicked();
    void on_browseToolButton_clicked();
    void loadSettings();
    void loadLanguages();
};

#endif // PREFERENCES_H
