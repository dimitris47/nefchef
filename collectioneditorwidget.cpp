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

#include "collectioneditorwidget.h"
#include "ingredientwidget.h"
#include <QCheckBox>
#include <QFile>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QIntValidator>
#include <QSaveFile>
#include <QTextCodec>
#include <QTextStream>
#include <QtMath>
#include <algorithm>

CollectionEditorWidget::CollectionEditorWidget(QWidget *parent) : CollectionPage(parent) {
    validator = new QIntValidator(0, 99999, this);
    caloriesGridLayout = new QGridLayout(this);
    caloriesGridLayout->setObjectName(QString::fromUtf8("verticalLayout"));
    this->setLayout(caloriesGridLayout);
}

void CollectionEditorWidget::updateDisplay() {
    setUpdatesEnabled(false);  // to avoid screen flicker

    auto caloriesWidgets = findChildren<IngredientWidget *>();
    for (auto &&widget : caloriesWidgets)
        widget->close();
    caloriesGridLayout->invalidate();

    int rowsPerColumn = qCeil(static_cast<float>(_tmpIngredients.size()) / columns());
    for (int i = 0; i < _tmpIngredients.size(); i++) {
        int column = (i / rowsPerColumn);
        int row = i % rowsPerColumn;
        auto ingr = _tmpIngredients.at(i);
        auto widget = new IngredientWidget(ingr, this);
        widget->setObjectName(QString::fromUtf8("ingLabel") + QString::number(i));
        widget->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred, QSizePolicy::Label));
        widget->setAttribute(Qt::WA_DeleteOnClose);
        widget->setCaloriesValidator(validator);
        if (row == 0)
            widget->setHeaderVisible(true);
        caloriesGridLayout->addWidget(widget, row, column * columns());
        connect(widget, &IngredientWidget::stateChanged,      this, &CollectionEditorWidget::stateUpdates);
        connect(widget, &IngredientWidget::ingredientChanged, this, [=](const Ingredient &ingr) {
            _tmpIngredients.replace(i, ingr);
            _modified = true;
        });
        if (i == _tmpIngredients.size() - 1)
            _lastWidget = widget;
    }
    setUpdatesEnabled(true);
}

void CollectionEditorWidget::addIngredient() {
    auto ingr = Ingredient("new", 0);
    addNew(ingr);
    _lastWidget->setFocus();
}

void CollectionEditorWidget::addNew(Ingredient ingr) {
    if (ingr.name() != "") {
        _tmpIngredients.append(ingr);
        updateDisplay();
        Ingredients::ingredients = _tmpIngredients;
        _modified = true;
        emit itemAdded(ingr.name());
        auto lines = _lastWidget->findChildren<QLineEdit *>();
        connect(lines.at(1), &QLineEdit::editingFinished, this, &CollectionEditorWidget::editorChanged);
    }
}

void CollectionEditorWidget::added() {
    auto lines = findChildren<QLineEdit *>();
    newName = lines.at(lines.count()-2)->text();
    emit itemAdded(newName);
    lines.at(lines.count()-1)->setFocus();
}

void CollectionEditorWidget::removeSelected() {
    QList<int> selections;
    auto caloriesWidgets = findChildren<IngredientWidget *>();
    if (caloriesWidgets.count() == 0)
        return;
    for (auto &&widget : caloriesWidgets)
        if (widget->isSelected())
            selections.append(caloriesWidgets.indexOf(widget));
    if (selections.count() == caloriesWidgets.count())
        selections.clear();
    else
        for (auto &&widget : caloriesWidgets)
            if (widget->isSelected()) {
                _tmpIngredients.removeOne(widget->ingredient());
                updateDisplay();
                Ingredients::ingredients = _tmpIngredients;
                _modified = true;
            }
    emit itemRemoved(selections);
}

void CollectionEditorWidget::moveUp() {
    auto caloriesWidgets = findChildren<IngredientWidget *>();
    int count {0};
    for (int i = 1; i < caloriesWidgets.count(); i++)
        if (caloriesWidgets.at(i)->isSelected())
            count++;
    if (count > 1) {
        emit itemClimbed(-1);
        return;
    }
    for (int i = 1; i < caloriesWidgets.count(); i++)
        if (caloriesWidgets.at(i)->isSelected()) {
            #if QT_VERSION >= 0x050E02
                _tmpIngredients.swapItemsAt(i, i-1);
            #else
                _tmpIngredients.swap(i, i-1);
            #endif
            Ingredients::ingredients = _tmpIngredients;
            updateDisplay();
            _modified = true;
            emit itemClimbed(i);
            auto boxes = findChildren<QCheckBox *>();
            boxes.at(i+boxes.count()/2-1)->setChecked(true);
        }
}

void CollectionEditorWidget::moveDown() {
    auto caloriesWidgets = findChildren<IngredientWidget *>();
    int count {0};
    for (int i = 0; i < caloriesWidgets.count()-1; i++)
        if (caloriesWidgets.at(i)->isSelected())
            count++;
    if (count > 1) {
        emit itemDescended(-1);
        return;
    }
    for (int i = 0; i < caloriesWidgets.count()-1; i++)
        if (caloriesWidgets.at(i)->isSelected()) {
            #if QT_VERSION >= 0x050E02
                _tmpIngredients.swapItemsAt(i, i+1);
            #else
                _tmpIngredients.swap(i, i+1);
            #endif
            Ingredients::ingredients = _tmpIngredients;
            updateDisplay();
            _modified = true;
            emit itemDescended(i);
            auto boxes = findChildren<QCheckBox *>();
            boxes.at(i+boxes.count()/2+1)->setChecked(true);
        }
}

void CollectionEditorWidget::stateUpdates() {
    IngredientWidget *w = qobject_cast<IngredientWidget *>(sender());
    auto box = w->findChildren<QCheckBox *>();
    auto theBox = box.at(0);
    auto boxes = findChildren<QCheckBox *>();
    for (int i = 0; i < boxes.count(); i++)
        if (boxes.at(i) == theBox)
            emit stateChanged(i);
}
