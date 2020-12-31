#ifndef MASSLINEEDIT_H
#define MASSLINEEDIT_H

#include <QLineEdit>

class MassLineEdit : public QLineEdit {
    Q_OBJECT
    Q_PROPERTY(int calories READ calories)

public:
    MassLineEdit(int calories, QWidget* parent = nullptr)
    : QLineEdit(parent), _calories(calories) {}
    int calories() const { return _calories; }

private:
    const int _calories;
};

#endif // MASSLINEEDIT_H
