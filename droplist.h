#ifndef DROPLIST_H
#define DROPLIST_H

#include <QDialog>
#include <QListWidget>
#include "ingredients.h"

namespace Ui {
class DropList;
}

class DropList : public QDialog {
    Q_OBJECT

public:
    explicit DropList(QWidget *parent = nullptr);
    ~DropList();
    QStringList selectedItems() const;
    void read(const QString &filename);

private slots:
    void on_listWidget_itemDoubleClicked(QListWidgetItem *item);
    void on_listWidget2_itemDoubleClicked();
    void on_deselectButton_clicked();
    void on_selectButton_clicked();

private:
    Ui::DropList *ui;
    QStringList combolist;
};

#endif // DROPLIST_H
