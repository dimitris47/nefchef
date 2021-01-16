#ifndef ADAPTOR_H
#define ADAPTOR_H

#include <QDialog>

namespace Ui { class Adaptor; }

class Adaptor : public QDialog {
    Q_OBJECT

public:
    explicit Adaptor(QWidget *parent = nullptr);
    ~Adaptor();
    double getFrac() const { return frac; }
    double getNum() const { return num; }
    double getDen() const { return den; }

private slots:
    void on_buttonBox_accepted();

private:
    Ui::Adaptor *ui;
    double frac;
    double num;
    double den;
};

#endif // ADAPTOR_H
