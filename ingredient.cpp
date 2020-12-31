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

#include "ingredient.h"

class IngredientData : public QSharedData {
public:
    IngredientData() {}
    IngredientData(const IngredientData &other)
        : QSharedData(other), name(other.name), calories(other.calories) {}
    ~IngredientData() {}

    QString name;
    int calories;
};

Ingredient::Ingredient() : d(new IngredientData) {
    d->name = QString("");
    d->calories = 0;
}

Ingredient::Ingredient(const QString &name, int calories) : d(new IngredientData) {
    d->name = name;
    d->calories = calories;
}

Ingredient::Ingredient(const Ingredient &rhs) : d(rhs.d) {}

Ingredient &Ingredient::operator=(const Ingredient &rhs) {
    if (this != &rhs)
        d.operator=(rhs.d);
    return *this;
}

Ingredient::~Ingredient() {}

void Ingredient::setName(const QString &name) {
    d->name = name;
}

void Ingredient::setCalories(int calories) {
    d->calories = calories;
}

QString Ingredient::name() const {
    return d->name;
}

int Ingredient::calories() const {
    return d->calories;
}

QDebug operator<<(QDebug debug, const Ingredient &ingr) {
    QDebugStateSaver saver(debug);
    debug.noquote() << "Ingredient(" << ingr.name() << ", " << ingr.calories() << ")";
    return debug;
}
