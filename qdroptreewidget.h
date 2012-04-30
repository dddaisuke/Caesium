#ifndef QDROPTREEWIDGET_H
#define QDROPTREEWIDGET_H

#include <QTreeWidget>

class QMimeData;

class QDropTreeWidget : public QTreeWidget
{
    Q_OBJECT

public:
    QDropTreeWidget(QWidget *parent = 0);

private slots:
    bool d_duplicateCheck(QString name, QString dir);
    void actionPreview();
    void actionRemove();
    void actionAdd();
    void actionAddFolder();
    void openDestination();
    void openInput();
    void actionRemoveFromHDD();
    void actionClear();

signals:
    void countUpdate();
    void preview();
    void remove();
    void add();
    void addFolder();
    void destination();
    void input();
    void removeHDD();
    void a_clear();

protected:
    void dragEnterEvent(QDragEnterEvent *event);
//    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);

private:
    void createActions();
    void createMenus();
    bool checkDestination();
    bool checkInput();

    QTreeWidget *dropTreeWidget();
    QMenu *itemMenu;
    QMenu *noItemMenu;
    QAction *clearAction;
    QAction *previewAction;
    QAction *removeAction;
    QAction *addAction;
    QAction *addFolderAction;
    QAction *destinationAction;
    QAction *originalAction;
    QAction *removeFromHDDAction;
};

#endif // QDROPTREEWIDGET_H
