#ifndef COMBO_H
#define COMBO_H

#include "ingredient.h"
#include <QDialog>

namespace Ui { class Combo; }

class Combo : public QDialog {
    Q_OBJECT

public:
    explicit Combo(QWidget *parent = nullptr);
    ~Combo();
    Ingredient getNewIng() const { return newIng; }

private slots:
    void on_addButton_clicked();

private:
    Ui::Combo *ui;
    Ingredient newIng;
};

#endif // COMBO_H
