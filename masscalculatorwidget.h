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

signals:
    void refresh();
    void refreshMasses(QStringList lastMasses);

public slots:
    void calculation();
    void doRefresh(float kcalsum, int masssum, float percentsum, QStringList names);
    void doRefreshMasses(float kcalsum, int masssum, float percentsum, QStringList names, QStringList lastMasses);

private slots:
    void clear();
    void on_refreshButton_clicked();

private:
    Ui::MassCalculatorWidget *ui;
    bool _modified { false };
};

#endif // MASSCALCULATORWIDGET_H
