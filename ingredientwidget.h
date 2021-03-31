#ifndef INGREDIENTWIDGET_H
#define INGREDIENTWIDGET_H

#include <QFrame>
#include "ingredient.h"
class QValidator;

namespace Ui { class IngredientWidget; }

class IngredientWidget : public QFrame {
    Q_OBJECT
    Q_PROPERTY(Ingredient ingredient READ ingredient WRITE setIngredient NOTIFY ingredientChanged)

public:
    explicit IngredientWidget(QWidget *parent = nullptr);
    explicit IngredientWidget(const Ingredient &ingr, QWidget *parent = nullptr);
    ~IngredientWidget() override;
    void setCaloriesValidator(const QValidator* validator);
    Ingredient ingredient() const;
    void setIngredient(const Ingredient& ingr);
    bool isSelected() const;
    void setFocus();
    void setHeaderVisible(bool visible);

signals:
    void ingredientChanged(const Ingredient& ingr);
    void stateChanged();

private slots:
    void on_checkBoxSelect_stateChanged(int arg1);
    void setText(const Ingredient& ingr);
    void setIngredientName(const QString &name);
    void setIngredientCalories(const QString &calories);

private:
    Ui::IngredientWidget *ui;
    Ingredient _ingredient;
};

#endif // INGREDIENTWIDGET_H
