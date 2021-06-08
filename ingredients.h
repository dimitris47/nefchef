#ifndef INGREDIENTS_H
#define INGREDIENTS_H

#include "ingredient.h"
#include <QList>

namespace Ingredients {
    bool loadList(const QStringList &list);
    QString errorString();

    extern QList<Ingredient> ingredients;
    extern QString _errorString;
}

#endif // INGREDIENTS_H
