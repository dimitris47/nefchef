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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "adaptor.h"
#include "collectioneditorwidget.h"
#include "combo.h"
#include "droplist.h"
#include "global.h"
#include "helpdialog.h"
#include "ingredientwidget.h"
#include "masscalculatorwidget.h"
#include "startpage.h"
#include <QActionGroup>
#include <QApplication>
#include <QCheckBox>
#include <QFile>
#include <QFileDialog>
#include <QFont>
#include <QFontDialog>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPrinter>
#include <QPushButton>
#include <QSaveFile>
#include <QScreen>
#include <QScrollArea>
#include <QScrollBar>
#include <QStackedWidget>
#include <QStandardPaths>
#include <QTextCodec>
#include <QTextDocument>
#include <QTextStream>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    start(new StartPage),
    editor(new CollectionEditorWidget),
    calculator(new MassCalculatorWidget)
{
    ui->setupUi(this);

    stackedWidget = new QStackedWidget(this);
    stackedWidget->setObjectName(QString::fromUtf8("stacked"));
    stackedWidget->addWidget(start);
    stackedWidget->addWidget(calculator);
    stackedWidget->addWidget(editor);

    QScrollArea *scrollArea  = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(stackedWidget);

    QPalette bgPlt = palette();
    bool bStatus = false;
    QString bgValue = "ffede2ff";
    uint bgHex = bgValue.toUInt(&bStatus, 16);
    bgPlt.setColor(QPalette::Background, QColor(bgHex));

    /* the following can be used instead of the previous four lines
     * in case a screen doesn't render custom colors (as it is with some HP laptop screens):
        bgPlt.setColor(QPalette::Background, QColor(Qt::darkCyan)); */

    scrollArea->setAutoFillBackground(true);
    scrollArea->setPalette(bgPlt);
    ui->toolBar->setAutoFillBackground(true);
    ui->toolBar->setPalette(bgPlt);
    ui->menuBar->setAutoFillBackground(true);
    ui->menuBar->setPalette(bgPlt);

    setCentralWidget(scrollArea);

    auto switchPages = new QActionGroup(this);
    switchPages->addAction(ui->actionStart);
    switchPages->addAction(ui->actionCalculator);
    switchPages->addAction(ui->actionEditor);

    auto editorActions = new QActionGroup(this);
    editorActions->addAction(ui->actionAddFromList);
    editorActions->addAction(ui->actionAddIngredient);
    editorActions->addAction(ui->actionRemove);
    editorActions->addAction(ui->actionMoveUp);
    editorActions->addAction(ui->actionMoveDown);
    editorActions->addAction(ui->actionAddColumn);
    editorActions->addAction(ui->actionRemoveColumn);
    editorActions->setEnabled(false);

    connect(stackedWidget, &QStackedWidget::currentChanged, this, [=](int page) {
        editorActions->setEnabled(page == 2);
    });
    connect(ui->actionAddIngredient, &QAction::triggered, editor, &CollectionEditorWidget::addIngredient);
    connect(ui->actionRemove,        &QAction::triggered, editor, &CollectionEditorWidget::removeSelected);
    connect(ui->actionAddColumn,     &QAction::triggered, this,   [=]() {
        setColumnNumber(editor->columns() + 1);
    });
    connect(ui->actionRemoveColumn,  &QAction::triggered, this,   [=]() {
        int cols = editor->columns();
        if (cols > 1)
            setColumnNumber(editor->columns() - 1);
    });
    connect(ui->actionStart,      &QAction::triggered, this, &MainWindow::showStart);
    connect(ui->actionCalculator, &QAction::triggered, this, &MainWindow::showCalculator);
    connect(ui->actionEditor,     &QAction::triggered, this, &MainWindow::showEditor);
    connect(ui->actionDrop,       &QAction::triggered, this, &MainWindow::showDropList);
    connect(ui->actionHelp,       &QAction::triggered, this, &MainWindow::helpPopup);
    connect(ui->actionInfo,       &QAction::triggered, this, &MainWindow::infoPopup);
    connect(ui->actionExit,       &QAction::triggered, this, &MainWindow::close);
    connect(ui->actionFont,       &QAction::triggered, this, &MainWindow::selectFont);
    connect(ui->actionMoveUp,     &QAction::triggered, this, [this]() { editor->moveUp(); });
    connect(ui->actionMoveDown,   &QAction::triggered, this, [this]() { editor->moveDown(); });

    ui->actionToggleToolbar->setChecked(true);
    readSettings();

    connect(calculator, &MassCalculatorWidget::refresh,         this, &MainWindow::refreshCalc);
    connect(calculator, &MassCalculatorWidget::refreshMasses,   this, &MainWindow::refreshCalcMasses);
    connect(editor,     &CollectionEditorWidget::editorChanged, this, [this]() {
        calculator->updateDisplay();
        updateExtendedList();
    });
    connect(editor,     &CollectionEditorWidget::itemAdded,     this, &MainWindow::addToCalc);
    connect(editor,     &CollectionEditorWidget::itemClimbed,   this, &MainWindow::calcClimb);
    connect(editor,     &CollectionEditorWidget::itemDescended, this, &MainWindow::calcDescend);
    connect(editor,     &CollectionEditorWidget::itemRemoved,   this, &MainWindow::calcRemove);
    connect(editor,     &CollectionEditorWidget::stateChanged,  this, &MainWindow::stateUpdates);
    connect(start,      &StartPage::create,                     this, [this]() { showDropList(); });
    connect(start,      &StartPage::help,                       this, [this]() { helpPopup(); });
    connect(start,      &StartPage::info,                       this, [this]() { infoPopup(); });
    connect(start,      &StartPage::open,                       this, &MainWindow::on_actionOpenRecipe_triggered);

    showStart();
    selMany = false;
}

MainWindow::~MainWindow() {
    delete start;
    delete editor;
    delete calculator;
    delete ui;
}

void MainWindow::refreshCalc() {
    auto lines = editor->findChildren<QLineEdit *>();
    auto widgets = editor->findChildren<IngredientWidget *>();

    QStringList calories;
    QStringList masses;
    QStringList names;
    QStringList updNames;

    for (int i = 1; i < lines.count(); i+=2)
        calories.append(lines.at(i)->text());
    for (auto &&line : calculator->findChildren<QLineEdit *>())
        masses.append(line->text());
    for (auto &&widget : widgets)
        names.append(widget->ingredient().name());

    int masssum {0};
    float kcalsum {0};
    for (int j = 0; j < masses.count(); j++) {
        masssum += masses.at(j).toInt();
        kcalsum += calories.at(j).toInt() * masses.at(j).toInt() / 100.0;
    }

    float percentsum {0};
    if (masssum)
        percentsum = kcalsum * 100 / masssum;

    calculator->doRefresh(kcalsum, masssum, percentsum, names);
}

void MainWindow::refreshCalcMasses(QStringList lastMasses) {
    auto lines = editor->findChildren<QLineEdit *>();
    auto widgets = editor->findChildren<IngredientWidget *>();

    QStringList calories;
    QStringList names;
    QStringList updNames;

    for (int i = 1; i < lines.count(); i+=2)
        calories.append(lines.at(i)->text());
    for (auto &&widget : widgets)
        names.append(widget->ingredient().name());

    int masssum {0};
    float kcalsum {0};
    for (int j = 0; j < lastMasses.count(); j++) {
        masssum += lastMasses.at(j).toInt();
        kcalsum += calories.at(j).toInt() * lastMasses.at(j).toInt() / 100.0;
    }

    float percentsum {0};
    if (masssum)
        percentsum = kcalsum * 100 / masssum;

    calculator->doRefreshMasses(kcalsum, masssum, percentsum, names, lastMasses);
}

void MainWindow::on_actionSelectMany_toggled(bool arg1) {
    selMany = arg1;
}

void MainWindow::stateUpdates(int boxNum) {
    if (!selMany) {
        auto boxes = editor->findChildren<QCheckBox *>();
        for (int i = 0; i < boxes.count(); i++)
            if (i != boxNum)
                boxes.at(i)->setCheckState(Qt::CheckState::Unchecked);
    }
}

void MainWindow::calcRemove(QList<int> selections) {
    QList<QLineEdit *> lines = calculator->findChildren<QLineEdit *>();
    QStringList masses;
    for (auto &&line : lines)
        masses.append(line->text());
    if (selections.count() == 0)
        statusBar()->showMessage(tr("Για αφαίρεση όλων των στοιχείων δημιουργήστε νέα συνταγή"));
    else
        for (int i = 0; i < selections.count(); i++)
            masses.removeAt(selections.at(i)-i);
    calculator->updateMasses(masses);
    calculator->calculation();
}

void MainWindow::addToCalc(QString name) {
    calculator->addIngr(name);
    calculator->calculation();
    updateExtendedList();
}

void MainWindow::calcClimb(int i) {
    if (i != -1) {
        QList<QLineEdit *> lines = calculator->findChildren<QLineEdit *>();
        QStringList masses;
        for (auto &&line : lines)
            masses.append(line->text());
        #if QT_VERSION >= 0x050E02
            masses.swapItemsAt(i, i-1);
        #else
            masses.swap(i, i-1);
        #endif
        calculator->updateMasses(masses);
        calculator->calculation();
    }
    else
        statusBar()->showMessage(tr("Δεν είναι δυνατή η ταυτόχρονη μετακίνηση πολλαπλών στοιχείων"));
}

void MainWindow::calcDescend(int i) {
    if (i >= 0) {
        QList<QLineEdit *> lines = calculator->findChildren<QLineEdit *>();
        QStringList masses;
        for (auto &&line : lines)
            masses.append(line->text());
        #if QT_VERSION >= 0x050E02
            masses.swapItemsAt(i, i+1);
        #else
            masses.swap(i, i+1);
        #endif
        calculator->updateMasses(masses);
        calculator->calculation();
    }
    else
        statusBar()->showMessage(tr("Δεν είναι δυνατή η ταυτόχρονη μετακίνηση πολλαπλών στοιχείων"));
}

void MainWindow::on_actionToggleToolbar_toggled(bool arg1) {
    ui->toolBar->setVisible(arg1);
}

void MainWindow::setColumnNumber(int columns) {
    editor->setColumns(columns);
    editor->updateDisplay();
    calculator->updateDisplay();
}

void MainWindow::selectFont() {
    QApplication::setFont(QFontDialog::getFont(0, QApplication::font()));
}

void MainWindow::showStart() {
    stackedWidget->setCurrentWidget(start);
    ui->actionAdaptor->setEnabled(false);
}

void MainWindow::showCalculator() {
    if (editor->isModified()) {
        QMessageBox box(QMessageBox::Warning,QApplication::applicationName(),
                        tr("Θέλετε να ενημερώσετε την τρέχουσα συνταγή με τις τελευταίες αλλαγές;\n"),
                        QMessageBox::Yes | QMessageBox::No,
                        this);
        box.setButtonText(QMessageBox::Yes, tr("Ναι"));
        box.setButtonText(QMessageBox::No, tr("Όχι"));

        const auto &ret = box.exec();
        switch (ret) {
        case QMessageBox::Yes:
            on_actionSaveRecipe_triggered();
            break;
        default:
            break;
        }
    }
    stackedWidget->setCurrentWidget(calculator);
    ui->actionAdaptor->setEnabled(true);
    calculator->updateDisplay();
}

void MainWindow::showEditor() {
    stackedWidget->setCurrentWidget(editor);
    ui->actionAdaptor->setEnabled(true);
}

void MainWindow::showDropList() {
    if (editor->isModified() || calculator->isModified()) {
        QMessageBox box(QMessageBox::Warning,QApplication::applicationName(),
                        tr("Υπάρχουν αλλαγές που δεν αποθηκεύτηκαν.\n"),
                        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
                        this);
        box.setButtonText(QMessageBox::Save, tr("Αποθήκευση"));
        box.setButtonText(QMessageBox::Discard, tr("Απόρριψη"));
        box.setButtonText(QMessageBox::Cancel, tr("Ακύρωση"));

        const auto &ret = box.exec();
        switch (ret) {
        case QMessageBox::Save:
            on_actionSaveRecipe_triggered();
            break;
        case QMessageBox::Cancel:
            return;
        default:
            break;
        }
    }

    DropList drop(this);
    int ret = drop.exec();
    if (ret == QDialog::Rejected)
        return;
    if (Ingredients::loadList(drop.selectedItems())) {
        editor->_tmpIngredients = Ingredients::ingredients;
        setColumnNumber(editor->columnsHint());
        setWindowTitle(QString("%1 - %2").arg(QApplication::applicationName(),
                       tr("[Προσωρινό Αρχείο]")));
        currentFile = ":/temp.rcp";
    }
    else
        statusBar()->showMessage(Ingredients::errorString(), 10000);
    showCalculator();
    ui->actionCalculator->setChecked(true);
    ui->actionStart->setChecked(false);
    editor->setModified(true);
}

void MainWindow::on_actionAddFromList_triggered() {
    Combo combo(this);
    int ret = combo.exec();
    if (ret == QDialog::Rejected) {
        auto widgets = editor->findChildren<QLineEdit *>();
        if (widgets.isEmpty())
            return;
    }
    editor->addNew(combo.getNewIng());
    editor->setModified(true);
}

void MainWindow::updateExtendedList() {
    QList<Ingredient> newIngr =  editor->_tmpIngredients - Ingredients::ingredients;

    QDir dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (!dataDir.exists())
        dataDir.mkpath(".");

    QFile comb(":/combined.cal");
    if (!comb.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in(&comb);
    in.setCodec(QTextCodec::codecForName("UTF-8"));
    QList<Ingredient> combIngr;
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList pair = line.split('=');
        QString name = pair.first();
        int calories = pair.last().toInt();
        Ingredient ingr;
        ingr.setName(name);
        ingr.setCalories(calories);
        combIngr.append(ingr);
    }
    comb.close();

    QFile ext(dataDir.path() + "/extended.cal");
    if (!ext.open(QIODevice::ReadOnly | QIODevice::Text))
        if (!ext.open(QIODevice::Append | QIODevice::Text))
            return;
    QTextStream extdata(&ext);
    extdata.setCodec(QTextCodec::codecForName("UTF-8"));
    QList<Ingredient> extIngr;
    while (!extdata.atEnd()) {
        QString extline = extdata.readLine();
        QStringList extpair = extline.split('=');
        QString extname = extpair.first();
        int extcalories = extpair.last().toInt();
        Ingredient extingr;
        extingr.setName(extname);
        extingr.setCalories(extcalories);
        extIngr.append(extingr);
    }
    ext.close();

    QString fileName = dataDir.path() + "/extended.cal";
    QFile file(fileName);
    if (!file.open(QIODevice::Append | QFile::Text)) {
        qWarning() << tr("error opening %1").arg(fileName);
        return;
    }
    QTextStream data(&file);
    data.setCodec(QTextCodec::codecForName("UTF-8"));
    data.setIntegerBase(10);
    for (auto &&ingredient : newIngr)
        if (!extIngr.contains(ingredient) && !combIngr.contains(ingredient))
            data << ingredient.name() << " = " << ingredient.calories() << '\n';
    file.close();
}

void MainWindow::on_actionAdaptor_triggered() {
    Adaptor *adaptor = new Adaptor;
    int ret = adaptor->exec();
    if (ret == QDialog::Rejected)
        return;

    auto lines = calculator->findChildren<QLineEdit *>();
    QList<int> masses;

    for (auto &&line : lines) {
        if (line->text().isNull())
            line->setText("0");
        else {
            if (!adaptor->getDen() || adaptor->getDen()==0 || !adaptor->getNum() || adaptor->getNum()==0) {
                statusBar()->showMessage(tr("Άκυρη μετατροπή"), 4000);
                return;
            }
            else {
                int newMass = line->text().toInt() * adaptor->getFrac();
                if (newMass == 0) {
                    QMessageBox box(QMessageBox::Warning,QApplication::applicationName(),
                                    tr("Μετά τη μετατροπή θα υπάρξουν συστατικά με μηδενική δοσολογία.\n"),
                                    QMessageBox::Cancel | QMessageBox::Ignore,
                                    nullptr);
                    box.setButtonText(QMessageBox::Cancel, tr("Ακύρωση"));
                    box.setButtonText(QMessageBox::Ignore, tr("Εντάξει"));

                    const auto &ret = box.exec();
                    switch (ret) {
                    case QMessageBox::Cancel:
                        return;
                    case QMessageBox::Ignore:
                        masses.append(0);
                        break;
                    default:
                        return;
                    }
                }
                else
                    masses.append(newMass);
            }
        }
    }
    for (int i = 0; i < masses.count(); i++)
        lines.at(i)->setText(QString::number(masses.at(i)));
    calculator->setModified(true);
}

QString writeableDir() {
    static QString dir;
    if (!dir.isEmpty())
        return dir;
    QStringList locations = (QStringList()
                             << QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation)
                             << QStandardPaths::standardLocations(QStandardPaths::HomeLocation));
    for (auto &&loc : locations)
        if (QFileInfo::exists(loc)) {
            dir = loc;
            return dir;
        }
    return QString();
}

bool MainWindow::saveRecipeFile(QStringList ingrs) {
    QString fileName = QFileDialog::getSaveFileName(this, tr("Αποθήκευση"), writeableDir(),
                                                    QString("Recipies (*.rcp);;Text files (*.txt);;All files (*.*)"));
    if (fileName.isEmpty() || fileName == QFileDialog::Rejected)
        return false;
    QFileInfo fi(fileName);
    if (fi.suffix().isEmpty())
        fileName += ".rcp";
    QSaveFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QFile::Text)) {
        qWarning() << tr("error opening %1").arg(fileName);
        return false;
    }
    QTextStream data(&file);
    data.setCodec(QTextCodec::codecForName("UTF-8"));
    data.setGenerateByteOrderMark(true);
    data.setIntegerBase(10);
    for (auto &&ingredient : ingrs)
        data << ingredient << '\n';
    if (data.status() != QTextStream::Ok) {
        qWarning() << tr("error saving %1").arg(fileName);
        return false;
    }
    file.commit();
    currentFile = fileName;
    setWindowTitle(QString("%1 - %2%3").arg(QApplication::applicationName(),
                   fi.fileName(),
                   fileName.startsWith(':') ? " [Preset]" : ""));
    return true;
}

bool MainWindow::on_actionSaveRecipe_triggered() {
    if (currentFile.isEmpty() || currentFile.startsWith(':')) {
        if (!on_actionSaveRecipeAs_triggered())
            return false;
    }
    else {
        if (editor->isModified())
            updateExtendedList();
        Ingredients::ingredients = editor->_tmpIngredients;
        QSaveFile file(currentFile);
        if (!file.open(QIODevice::WriteOnly | QFile::Text)) {
            qWarning() << QObject::tr("Σφάλμα ανοίγματος αρχείου: %1").arg(file.errorString());
            return false;
        }
        else {
            ingrs.clear();
            auto caloriesWidgets = editor->_tmpIngredients;
            auto masses = calculator->findChildren<QLineEdit *>();
            QList<int> kcalList;
            QStringList labelData;
            QStringList lineData;
            for (auto &&widget : caloriesWidgets) {
                labelData.append(widget.name().replace('=', ':').replace('>', ':'));
                kcalList.append(widget.calories());
            }
            for (auto &&mass : masses)
                lineData.append(mass->text());
            if (lineData.count()!=labelData.count())
                return false;
            for (int i=0; i<labelData.count(); i++) {
                QString ingr = labelData[i] + " > " + QString::number(kcalList[i]) + " > " + lineData[i];
                ingrs.append(ingr);
            }
            ingrs.append("#\n" + calculator->instruct->toPlainText());

            QTextStream data(&file);
            data.setCodec(QTextCodec::codecForName("UTF-8"));
            data.setGenerateByteOrderMark(true);
            data.setIntegerBase(10);
            for (auto &&ingredient : ingrs)
                data << ingredient << '\n';
            if (data.status() != QTextStream::Ok) {
                qWarning() << tr("error saving %1").arg(currentFile);
                return false;
            }
            file.commit();
            calculator->updateDisplay();
            calculator->calculation();
            editor->setModified(false);
            calculator->setModified(false);
            statusBar()->showMessage(Ingredients::errorString(), 5000);
            ingrs.clear();
        }
    }
    return true;
}

bool MainWindow::on_actionSaveRecipeAs_triggered() {
    if (editor->_tmpIngredients.isEmpty()) {
        statusBar()->showMessage(tr("Δεν υπάρχει ανοιχτή συνταγή για αποθήκευση"), 3000);
        return false;
    }
    else {
        ingrs.clear();
        auto caloriesWidgets = editor->_tmpIngredients;
        auto masses = calculator->findChildren<QLineEdit *>();
        QList<int> kcalList;
        QStringList labelData;
        QStringList lineData;
        for (auto &&widget : caloriesWidgets) {
            labelData.append(widget.name().replace('=', ':').replace('>', ':'));
            kcalList.append(widget.calories());
        }
        for (auto &&mass : masses)
            lineData.append(mass->text());
        if (lineData.count()!=labelData.count())
            return false;
        for (int i = 0; i < labelData.count(); i++) {
            QString ingr = labelData[i] + " > " + QString::number(kcalList[i]) + " > " + lineData[i];
            ingrs.append(ingr);
        }
        ingrs.append("#\n" + calculator->instruct->toPlainText());
        if (saveRecipeFile(ingrs)) {
            editor->setModified(false);
            calculator->setModified(false);
            ingrs.clear();
            on_actionSaveRecipe_triggered();
            return true;
        }
    }
    return false;
}

void MainWindow::on_actionOpenRecipe_triggered() {
    if (editor->isModified() || calculator->isModified()) {
        QMessageBox box(QMessageBox::Warning,QApplication::applicationName(),
                        tr("Υπάρχουν αλλαγές που δεν αποθηκεύτηκαν.\n"),
                        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
                        this);
        box.setButtonText(QMessageBox::Save, tr("Αποθήκευση"));
        box.setButtonText(QMessageBox::Discard, tr("Απόρριψη"));
        box.setButtonText(QMessageBox::Cancel, tr("Ακύρωση"));

        const auto &ret = box.exec();
        switch (ret) {
        case QMessageBox::Save:
            on_actionSaveRecipe_triggered();
            break;
        case QMessageBox::Cancel:
            return;
        default:
            break;
        }
    }
    instr.clear();
    QString fileName = QFileDialog::getOpenFileName(this, tr("Άνοιγμα αρχείου"), writeableDir(),
                                                    QString("Recipies (*.rcp);;Text files (*.txt);;All files (*.*)"));
    if (fileName.isEmpty())
        return;
    QFileInfo fi(fileName);
    setWindowTitle(QString("%1 - %2").arg(QApplication::applicationName(), fi.fileName()));

    QFile data(fileName);
    if (!data.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << tr("error opening %1").arg(fileName);
        return;
    }
    QTextStream reader(&data);
    reader.setCodec(QTextCodec::codecForName("UTF-8"));
    while (!reader.atEnd()) {
        QString line = reader.readLine();
        if (line.startsWith("#"))
            break;
        recipeIngrs.append(line);
    }
    while (!reader.atEnd()) {
        QString line = reader.readLine();
        if (!recipeIngrs.contains(line))
            instr.append(line);
    }
    openRecipe(fileName);
    currentFile = fileName;
    recipeIngrs.clear();
}

void MainWindow::openRecipe(const QString &fileName) {
    QStringList masses;
    editor->_tmpIngredients.clear();
    Ingredients::ingredients.clear();
    for (auto &&ingr : recipeIngrs) {
        QStringList items = ingr.split(" > ");
        QString name = items[0];
        int calories = items[1].toInt();
        QString mass = items[2];
        Ingredient *newIngr = new Ingredient(name, calories);
        editor->_tmpIngredients.append(*newIngr);
        Ingredients::ingredients.append(*newIngr);
        masses.append(mass);
    }

    auto caloriesWidgets = findChildren<IngredientWidget *>();
    for (auto &&widget : caloriesWidgets)
        widget->close();
    editor->setColumns(1);
    editor->updateDisplay();
    calculator->setColumns(1);
    calculator->updateDisplay();

    auto lines = calculator->findChildren<QLineEdit *>();
    for (int i = 0; i < masses.count(); i++)
        lines[lines.count() - masses.count() + i]->setText(masses[i]);
    calculator->calculation();

    QFileInfo fi(fileName);
    currentFile = fileName;
    setWindowTitle(QString("%1 - %2").arg(QApplication::applicationName(), fi.fileName()));

    calculator->instruct->setPlainText(instr.join("\n"));
    calculator->instruct->verticalScrollBar()->setValue(0);
    stackedWidget->setCurrentWidget(calculator);
    ui->actionCalculator->setChecked(true);
    ui->actionStart->setChecked(false);
    editor->setModified(false);
    calculator->setModified(false);
}

void MainWindow::on_action_export_to_pdf_triggered() {
    auto caloriesWidgets = editor->_tmpIngredients;
    QList<int> kcalList;
    QStringList labelData;
    QStringList lineData;
    auto labelsList = calculator->findChildren<QLabel *>();
    auto linesList = calculator->findChildren<QLineEdit *>();
    for (auto &&line : linesList)
        if (!line->text().isEmpty()) {
            lineData.append(line->text());
            labelData.append(labelsList[linesList.indexOf(line)+editor->columns()+6]->text());
        }
    for (auto &&widget : caloriesWidgets)
        for (auto &&label : labelData)
            if (widget.name().startsWith(label))
                kcalList.append(widget.calories());

    QString fileName = QFileDialog::getSaveFileName(nullptr, "Export PDF", writeableDir() + currentFile.remove(".rcp"), "*.pdf");
    if (fileName.isEmpty())
        return;
    if (QFileInfo(fileName).suffix().isEmpty())
        fileName.append(".pdf");
    QPrinter printer(QPrinter::PrinterResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setOutputFileName(fileName);

    QStringList ingrList;
    for (int i = 0; i < labelData.count(); i++) {
        QString ingr = lineData[i] + " γρ. " + labelData[i];
        ingrList.append("<span>&#8226; " + ingr + "</span>");
    }

    QStringList instrList;
    for (auto &&line : calculator->instruct->toPlainText().replace("<", "&#60;").split("\n")) {
        line.isEmpty() ? instrList.append(line) : instrList.append("<span>&#8226; " + line + "</span>");
    }

    QTextDocument doc;
    QFileInfo fi(fileName);

    QString stdText = "<p style='text-align: right'>Σύνολο: " + labelsList[4]->text() + "<br/>" + labelsList[5]->text() + "</p>" \
                + "<p style='text-align: center'><b><h2>" + fi.baseName() + "</b></h2></p>" \
                + "<p style='line-height:120%'><br/><u>Υλικά:</u><br/>" + ingrList.join("<br/>") + "</p><br/>";
    QString instrText = "<p style='line-height:120%'><u>Οδηγίες εκτέλεσης:</u><br/>" + instrList.join("<br/>") + "</p>";
    QString fullText = stdText + instrText;

    !calculator->instruct->toPlainText().isEmpty() ? doc.setHtml(fullText) : doc.setHtml(stdText);
    doc.print(&printer);
    ingrList.clear();
}

void MainWindow::helpPopup() {
    QFile file(":/instructions.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    QTextStream in(&file);
    in.setCodec(QTextCodec::codecForName("UTF-8"));
    QString text = in.readAll();
    file.close();

    auto screen = qApp->screens().first();
    QSize s = screen->availableSize();
    HelpDialog dialog;
    dialog.setText(text);
    dialog.resize(s.width()*0.5, s.height()*0.5);
    dialog.exec();
}

void MainWindow::infoPopup() {
    QMessageBox::about(this, tr("Πληροφορίες προγράμματος"),
                       (QApplication::applicationName() + " " + QApplication::applicationVersion() + "<br/><br/>" +
                        APPINFO));
}

void MainWindow::readSettings() {
    QSettings settings;
    bool isMax = settings.value("isMaximized", false).toBool();
    if (isMax) {
        showMaximized();
    } else {
        const QByteArray geometry = settings.value("geometry", QByteArray()).toByteArray();
        restoreGeometry(geometry);
    }
    const QString f = settings.value("font", QFont()).toString();
    const int s = settings.value("size", 11).toInt();
    const QFont font(f, s);
    QApplication::setFont(font);
}

void MainWindow::closeEvent(QCloseEvent *event) {
    updateExtendedList();
    if (editor->isModified() || calculator->isModified()) {
        QMessageBox box(QMessageBox::Warning,QApplication::applicationName(),
                        tr("Υπάρχουν αλλαγές που δεν αποθηκεύτηκαν.\n"),
                        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
                        this);
        box.setButtonText(QMessageBox::Save, tr("Αποθήκευση"));
        box.setButtonText(QMessageBox::Discard, tr("Απόρριψη"));
        box.setButtonText(QMessageBox::Cancel, tr("Ακύρωση"));

        const auto &ret = box.exec();
        switch (ret) {
        case QMessageBox::Save:
            if (!on_actionSaveRecipe_triggered()) {
                event->ignore();
                return;
            } else {
                event->accept();
            }
            break;
        case QMessageBox::Cancel:
            event->ignore();
            return;
        default:
            break;
        }
    }
    {
        QSettings settings;
        settings.setValue("isMaximized", isMaximized());
        if (!isMaximized())
            settings.setValue("geometry", saveGeometry());
        settings.setValue("font", QApplication::font().toString());
        settings.setValue("size", QApplication::font().pointSize());
    }
    updateExtendedList();
    event->accept();
}
