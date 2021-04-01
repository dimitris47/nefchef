#ifndef COLLECTIONPAGE_H
#define COLLECTIONPAGE_H

#include "ingredients.h"
#include <QList>
#include <QWidget>
#include <QtMath>

class CollectionPage : public QWidget {
    Q_OBJECT
    
public:
    explicit CollectionPage(QWidget *parent = nullptr) : QWidget(parent) {}
    virtual ~CollectionPage() {}
    int columnsHint() const {
        return qCeil(static_cast<float>(Ingredients::ingredients.size()) / 15);
    }
    static int columns() { return _columns; }
    static void setColumns(int columns) { _columns = columns; }
    
protected:
    virtual void updateDisplay() = 0;
    
private:
    static int _columns;
};

#endif // COLLECTIONPAGE_H
