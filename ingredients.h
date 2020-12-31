#ifndef INGREDIENTS_H
#define INGREDIENTS_H

#include "ingredient.h"
#include <QList>

namespace Ingredients {
    bool loadData(const QString &fileName);
    bool loadList(const QStringList &list);
    bool saveData(const QString &fileName);
    QString errorString();

    extern QList<Ingredient> ingredients;
    extern QString _errorString;
    extern const QString preamble;
}

#endif // INGREDIENTS_H
