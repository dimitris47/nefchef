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

#include "droplist.h"
#include "ui_droplist.h"
#include <QAbstractItemView>
#include <QCompleter>
#include <QDir>
#include <QFile>
#include <QList>
#include <QListView>
#include <QSaveFile>
#include <QStandardPaths>
#include <QTextCodec>
#include <QTextStream>

DropList::DropList(QWidget *parent) : QDialog(parent), ui(new Ui::DropList) {
    ui->setupUi(this);
    read(":/combined.cal");
    QDir dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (dataDir.exists()) {
        QFile ext(dataDir.path() + "/extended.cal");
        if (!ext.open(QIODevice::ReadOnly | QIODevice::Text))
            return;
        QTextStream extdata(&ext);
        extdata.setCodec(QTextCodec::codecForName("UTF-8"));
        QStringList extIngr;
        while (!extdata.atEnd()) {
            QString extline = extdata.readLine();
            extIngr.append(extline);
        }
        for (auto &&line : extIngr)
            if (!combolist.contains(line)) {
                ui->listWidget->addItem(line);
                combolist.append(line);
            }
    }
    auto completer = new QCompleter(combolist, Q_NULLPTR);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
}

DropList::~DropList() { delete ui; }

QStringList DropList::selectedItems() const {
    QStringList selected;
    for (int row=0; row< ui->listWidget2->count(); row++ )
        selected << ui->listWidget2->item(row)->text();
    return selected;
}

void DropList::read(const QString& filename) {
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    QTextStream reader(&file);
    reader.setCodec(QTextCodec::codecForName("UTF-8"));
    while (!reader.atEnd()) {
        QString line = reader.readLine();
        ui->listWidget->addItem(line);
        combolist.append(line);
    }
}

void DropList::on_listWidget_itemDoubleClicked(QListWidgetItem *item) {
    ui->listWidget2->addItem(item->text());
}

void DropList::on_selectButton_clicked() {
    ui->listWidget2->addItem(ui->listWidget->currentItem()->text());
}

void DropList::on_listWidget2_itemDoubleClicked() {
    auto item = ui->listWidget2->takeItem(ui->listWidget2->currentRow());
    delete item;
}

void DropList::on_deselectButton_clicked() {
    auto item = ui->listWidget2->takeItem(ui->listWidget2->currentRow());
    delete item;
}
