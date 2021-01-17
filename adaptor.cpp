/**
 * Copyright 2020 Dimitris Psathas <dimitrisinbox@gmail.com>
 *
 * This file is part of NefChef.
 *
 * NefChef is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License  as  published by  the  Free Software
 * Foundation,  either version 3 of the License,  or (at your option)  any later
 * version.
 *
 * NefChef is distributed in the hope that it will be useful,  but  WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the  GNU General Public License  for more details.
 *
 * You should have received a copy of the  GNU General Public License along with
 * NefChef. If not, see <http://www.gnu.org/licenses/>.
 */

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
