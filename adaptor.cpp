#include "adaptor.h"
#include "ui_adaptor.h"

Adaptor::Adaptor(QWidget *parent) : QDialog(parent), ui(new Ui::Adaptor) {
    ui->setupUi(this);
}

Adaptor::~Adaptor() { delete ui; }

void Adaptor::on_buttonBox_accepted() {
    frac = ui->den->text().toDouble() / ui->num->text().toDouble();
}
