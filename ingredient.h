#ifndef INGREDIENT_H
#define INGREDIENT_H

#include <QDebug>
#include <QList>
#include <QSharedDataPointer>

class IngredientData;

class Ingredient {
public:
    Ingredient();
    Ingredient(const QString &name, int calories);
    Ingredient(const Ingredient&);
    Ingredient &operator=(const Ingredient&);
    ~Ingredient();

    void setName(const QString &name);
    void setCalories(int calories);

    QString name() const;
    int calories() const;

private:
    QSharedDataPointer<IngredientData> d;
};

inline bool operator==(const Ingredient &lhs, const Ingredient &rhs) {
    return lhs.name().toLower() == rhs.name().toLower() && lhs.calories() == rhs.calories();
}

inline bool operator<(const Ingredient &lhs, const Ingredient &rhs) {
    return lhs.name() < rhs.name();
}

inline QList<Ingredient> operator-(const QList<Ingredient> &lhs, const QList<Ingredient> &rhs) {
    QList<Ingredient> ingr {};
    for (auto i : lhs)
        if (!rhs.contains(i))
            ingr << i;
    return ingr;
}

QDebug operator<<(QDebug debug, const Ingredient &ingr);
Q_DECLARE_TYPEINFO(Ingredient, Q_MOVABLE_TYPE);

#endif // INGREDIENT_H
