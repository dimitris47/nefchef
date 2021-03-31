#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ingredient.h"
#include "ingredientwidget.h"
#include <QCloseEvent>
#include <QMainWindow>
#include <QSettings>

class StartPage;
class CollectionEditorWidget;
class MassCalculatorWidget;
class QStackedWidget;

namespace Ui { class MainWindow; }

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void loadFile(const QString &fileName);
    void openRecipe(const QString &fileName);
    bool saveFile(const QString &fileName);

public slots:
    void addToCalc(QString name);
    void calcClimb(int i);
    void calcDescend(int i);
    void calcRemove(QList<int> selections);
    void startCreate();
    void startHelp();
    void startInfo();
    void startOpen();
    void stateUpdates(int boxNum);
    void updateCalc();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    bool saveRecipeFile(QStringList ingrs);
    void readSettings();
    void selectFont();
    void setColumnNumber(int columns);
    void updateExtendedList();
    Ui::MainWindow *ui;
    StartPage *start;
    CollectionEditorWidget *editor;
    MassCalculatorWidget *calculator;
    QStackedWidget *stackedWidget;
    QString currentFile;
    QStringList ingrs;
    QStringList instr;
    QStringList recipeIngrs;

private slots:
    bool on_actionSaveRecipe_triggered();
    bool on_actionSaveRecipeAs_triggered();
    void helpPopup();
    void infoPopup();
    void on_actionAdaptor_triggered();
    void on_actionAddFromList_triggered();
    void on_action_export_to_pdf_triggered();
    void on_actionMoveUp_triggered();
    void on_actionMoveDown_triggered();
    void on_actionOpenRecipe_triggered();
    void on_actionToggleToolbar_toggled(bool arg1);
    void showCalculator();
    void showEditor();
    void showDropList();
    void showStart();
};

#endif // MAINWINDOW_H
