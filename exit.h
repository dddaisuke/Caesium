#ifndef EXIT_H
#define EXIT_H

#include <QDialog>

namespace Ui {
    class Exit;
}

class Exit : public QDialog
{
    Q_OBJECT

public:
    explicit Exit(QWidget *parent = 0);
    ~Exit();

private slots:

    void on_alwaysCheckBox_toggled(bool checked);

private:
    Ui::Exit *ui;
    QWidget *e_parent;
};

#endif // EXIT_H
