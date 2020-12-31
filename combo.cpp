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

#include "combo.h"
#include "ui_combo.h"
#include "ingredient.h"
#include <QAbstractItemView>
#include <QCompleter>
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QTextCodec>
#include <QTextStream>

Combo::Combo(QWidget *parent) : QDialog(parent), ui(new Ui::Combo) {
    ui->setupUi(this);
    QFile file(":/combined.cal");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    QTextStream reader(&file);
    reader.setCodec(QTextCodec::codecForName("UTF-8"));
    QStringList combolist;
    while (!reader.atEnd()) {
        QString line = reader.readLine();
        combolist.append(line);
    }
    QDir dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (dataDir.exists()) {
        QFile ext(dataDir.path() + "/extended.cal");
        if (ext.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream extdata(&ext);
            extdata.setCodec(QTextCodec::codecForName("UTF-8"));
            QStringList extIngr;
            while (!extdata.atEnd()) {
                QString extline = extdata.readLine();
                extIngr.append(extline);
            }
            for (QString line : extIngr)
                if (!combolist.contains(line)) {
                    combolist.append(line);
                }
        }
    }
    combolist.sort();
    for (QString item : combolist)
        ui->comboBox->addItem(item);

    auto completer = new QCompleter(combolist, Q_NULLPTR);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
}

Combo::~Combo() { delete ui; }

void Combo::on_addButton_clicked() {
    QString selected = ui->comboBox->currentText();
    QRegularExpression tagExp(" = ");
    QStringList line = selected.split(tagExp);
    QString name = line.first();
    int calories = line.last().toInt();
    newIng.setName(name);
    newIng.setCalories(calories);
    this->close();
}
