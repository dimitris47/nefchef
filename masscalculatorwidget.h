#ifndef MASSCALCULATORWIDGET_H
#define MASSCALCULATORWIDGET_H

#include "collectionpage.h"
#include <QPlainTextEdit>
#include <QWidget>
class MassLineEdit;

namespace Ui { class MassCalculatorWidget; }

class MassCalculatorWidget : public CollectionPage {
    Q_OBJECT

public:
    explicit MassCalculatorWidget(QWidget *parent = nullptr);
    ~MassCalculatorWidget();
    void addIngr(QString name);
    void updateDisplay() override;
    void updateMasses(QStringList masses);
    QPlainTextEdit *instruct;
    inline bool isModified() const { return _modified; }
    inline void setModified(bool modified) { _modified = modified; }
    QList<MassLineEdit *> lineEdits {};

public slots:
    void calculation();

private slots:
    void clear();
    
private:
    Ui::MassCalculatorWidget *ui;
    bool _modified { false };
};

#endif // MASSCALCULATORWIDGET_H
