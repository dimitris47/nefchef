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

#include "startpage.h"
#include "ui_startpage.h"

StartPage::StartPage(QWidget *parent) : QWidget(parent), ui(new Ui::StartPage) {
    ui->setupUi(this);
}

StartPage::~StartPage() { delete ui; }

void StartPage::on_startOpen_clicked() { emit open(); }

void StartPage::on_startCreate_clicked() { emit create(); }

void StartPage::on_startHelp_clicked() { emit help(); }

void StartPage::on_startInfo_clicked() { emit info(); }
