#include "adaptor.h"
#include "ui_adaptor.h"
#include <QDoubleValidator>

Adaptor::Adaptor(QWidget *parent) : QDialog(parent), ui(new Ui::Adaptor) {
    ui->setupUi(this);
    QDoubleValidator *validator = new QDoubleValidator(0.01, 99.99, 2, this);
    ui->num->setValidator(validator);
    ui->den->setValidator(validator);
}

Adaptor::~Adaptor() { delete ui; }

void Adaptor::on_buttonBox_accepted() {
    den = ui->den->text().toDouble();
    num = ui->num->text().toDouble();
    if (!den || den==0 || !num || num==0)
        return;
    else
        frac = ui->den->text().toDouble() / ui->num->text().toDouble();
}
