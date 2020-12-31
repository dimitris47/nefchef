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

#include "ingredientwidget.h"
#include "ui_ingredientwidget.h"

IngredientWidget::IngredientWidget(QWidget *parent)
    : IngredientWidget(Ingredient(), parent)
{}

IngredientWidget::IngredientWidget(const Ingredient& ingr, QWidget *parent) :
    QFrame(parent),
    ui(new Ui::IngredientWidget),
    _ingredient(ingr)
{
    ui->setupUi(this);
    ui->labelHeader->setVisible(false);
    ui->lineEditName->setText(_ingredient.name());
    ui->lineEditCalories->setText(QString::number(_ingredient.calories()));
    connect(ui->lineEditName,     &QLineEdit::textEdited, this, &IngredientWidget::setIngredientName);
    connect(ui->lineEditCalories, &QLineEdit::textEdited, this, &IngredientWidget::setIngredientCalories);
}

IngredientWidget::~IngredientWidget() { delete ui; }

void IngredientWidget::setCaloriesValidator(const QValidator *validator) {
    ui->lineEditCalories->setValidator(validator);
}

Ingredient IngredientWidget::ingredient() const {
    return _ingredient;
}

void IngredientWidget::setIngredientName(const QString& name) {
    _ingredient.setName(name);
    emit ingredientChanged(_ingredient);
}

void IngredientWidget::setIngredientCalories(const QString& calories) {
    _ingredient.setCalories(calories.toInt());
    emit ingredientChanged(_ingredient);
}

void IngredientWidget::setIngredient(const Ingredient &ingr) {
    _ingredient = ingr;
}

bool IngredientWidget::isSelected() const {
    return ui->checkBoxSelect->isChecked();
}

void IngredientWidget::setFocus() {
    ui->lineEditName->setFocus();
}

void IngredientWidget::setHeaderVisible(bool visible) {
    ui->labelHeader->setVisible(visible);
}

void IngredientWidget::setText(const Ingredient &ingr) {
    ui->lineEditName->setText(ingr.name());
    ui->lineEditCalories->setText(QString::number(ingr.calories()));
}
