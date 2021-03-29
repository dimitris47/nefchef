#ifndef COLLECTIONEDITORWIDGET_H
#define COLLECTIONEDITORWIDGET_H

#include "collectionpage.h"
#include "ingredient.h"
#include "mainwindow.h"

class QGridLayout;
class QIntValidator;
class IngredientWidget;

class CollectionEditorWidget : public CollectionPage {
    Q_OBJECT

public:
    explicit CollectionEditorWidget(QWidget *parent = nullptr);
    ~CollectionEditorWidget() override {}
    void updateDisplay() override;
    void added();
    void addNew(Ingredient);
    inline bool isModified() const { return _modified; }
    inline void setModified(bool modified) { _modified = modified; }
    IngredientWidget* lastWidget() const { return _lastWidget; }

signals:
    void editorChanged();
    void itemAdded(QString name);
    void itemClimbed(int i);
    void itemDescended(int i);
    void itemRemoved(QList<int> selections);

public slots:
    void addIngredient();
    void moveDown();
    void moveUp();
    void removeSelected();

private:
    QGridLayout* caloriesGridLayout;
    QIntValidator* validator;
    IngredientWidget* _lastWidget { nullptr };
    QString newName;
    bool _modified { false };
    
public:
    QList<Ingredient> _tmpIngredients {};
};

#endif // COLLECTIONEDITORWIDGET_H
