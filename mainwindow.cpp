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
#include "helpdialog.h"
#include "ingredientwidget.h"
#include "masscalculatorwidget.h"
#include "startpage.h"
#include <QActionGroup>
#include <QApplication>
#include <QFile>
#include <QFileDialog>
#include <QFont>
#include <QFontDialog>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPrinter>
#include <QSaveFile>
#include <QScreen>
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
    setCentralWidget(stackedWidget);

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

    connect(stackedWidget,           &QStackedWidget::currentChanged, [=](int page) {
        editorActions->setEnabled(page == 2);
    });
    connect(ui->actionAddIngredient, &QAction::triggered, editor, &CollectionEditorWidget::addIngredient);
    connect(ui->actionRemove,        &QAction::triggered, editor, &CollectionEditorWidget::removeSelected);
    connect(ui->actionAddColumn,     &QAction::triggered, this,   [=]() {
        setColumnNumber(editor->columns() + 1);
    });
    connect(ui->actionRemoveColumn,  &QAction::triggered, this, [=]() {
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

    ui->actionToggleToolbar->setChecked(true);
    readSettings();

    connect(editor, &CollectionEditorWidget::editorChanged, this, &MainWindow::updateCalc);
    connect(editor, &CollectionEditorWidget::itemAdded,     this, &MainWindow::addToCalc);
    connect(editor, &CollectionEditorWidget::itemClimbed,   this, &MainWindow::calcClimb);
    connect(editor, &CollectionEditorWidget::itemDescended, this, &MainWindow::calcDescend);
    connect(editor, &CollectionEditorWidget::itemRemoved,   this, &MainWindow::calcRemove);
    connect(start,  &StartPage::create,                     this, &MainWindow::startCreate);
    connect(start,  &StartPage::help,                       this, &MainWindow::startHelp);
    connect(start,  &StartPage::info,                       this, &MainWindow::startInfo);
    connect(start,  &StartPage::open,                       this, &MainWindow::startOpen);
    showStart();
}

MainWindow::~MainWindow() {
    delete start;
    delete editor;
    delete calculator;
    delete ui;
}

void MainWindow::calcRemove(QList<int> selections) {
    QList<QLineEdit *> lines = calculator->findChildren<QLineEdit *>();
    QStringList masses;
    for (auto line : lines)
        masses.append(line->text());
    if (selections.count() == 0)
        statusBar()->showMessage(tr("Για αφαίρεση όλων των στοιχείων δημιουργήστε νέα συνταγή"));
    else
        for (int i=0; i<selections.count(); i++)
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
    if (i!=-1) {
        QList<QLineEdit *> lines = calculator->findChildren<QLineEdit *>();
        QStringList masses;
        for (auto line : lines)
            masses.append(line->text());
        #if QT_VERSION >= 0x050C00
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
    if (i>=0) {
        QList<QLineEdit *> lines = calculator->findChildren<QLineEdit *>();
        QStringList masses;
        for (auto line : lines)
            masses.append(line->text());
        #if QT_VERSION >= 0x050C00
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

void MainWindow::updateCalc() { calculator->updateDisplay(); }

void MainWindow::startOpen() { on_actionOpenRecipe_triggered(); }

void MainWindow::startCreate() { showDropList(); }

void MainWindow::startHelp() { helpPopup(); }

void MainWindow::startInfo() { infoPopup(); }

void MainWindow::on_actionToggleToolbar_toggled(bool arg1) {
    if (arg1==true)
        ui->toolBar->setVisible(true);
    else
        ui->toolBar->setVisible(false);
}

void MainWindow::setColumnNumber(int columns) {
    editor->setColumns(columns);
    editor->updateDisplay();
    calculator->updateDisplay();
}

void MainWindow::selectFont() {
    QApplication::setFont(QFontDialog::getFont(0, QApplication::font()));
}

void MainWindow::showStart() { stackedWidget->setCurrentWidget(start); }

void MainWindow::showCalculator() { stackedWidget->setCurrentWidget(calculator); }

void MainWindow::showEditor() { stackedWidget->setCurrentWidget(editor); }

void MainWindow::showDropList() {
    if (editor->isModified() || calculator->isModified()) {
        const QMessageBox::StandardButton ret
            = QMessageBox::warning(this, QApplication::applicationName(),
                                   tr("Υπάρχουν αλλαγές που δεν αποθηκεύτηκαν.\n"),
                                   QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
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

    DropList drop;
    int ret = drop.exec();
    if (ret == QDialog::Rejected)
        return;
    if (Ingredients::loadList(drop.selectedItems())) {
        editor->_tmpIngredients = Ingredients::ingredients;
        setColumnNumber(editor->columnsHint());
        setWindowTitle(QString("%1 - %2").arg(QApplication::applicationName())
                       .arg(tr("[Προσωρινό Αρχείο]")));
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
    Combo combo;
    combo.exec();
    currentFile = ":/temp.rcp";
    editor->addNew(combo.getNewIng());
    editor->added();
    editor->setModified(true);
}

void MainWindow::openRecipe(const QString &fileName) {
    QStringList masses;
    editor->_tmpIngredients.clear();
    Ingredients::ingredients.clear();
    for (const QString &ingr : recipeIngrs) {
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
    for (auto widget : caloriesWidgets)
        widget->close();
    editor->setColumns(1);
    editor->updateDisplay();
    calculator->setColumns(1);
    calculator->updateDisplay();

    auto lines = calculator->findChildren<QLineEdit *>();
    for (int i=0; i<masses.count(); i++)
        lines[lines.count()-masses.count()+i]->setText(masses[i]);
    calculator->calculation();

    QFileInfo fi(fileName);
    currentFile = fileName;
    setWindowTitle(QString("%1 - %2%3").arg(QApplication::applicationName())
                   .arg(fi.fileName())
                   .arg(fileName.startsWith(':') ? " [Preset]" : ""));

    calculator->instruct->setPlainText(instr.join("\n"));
    showCalculator();
    ui->actionCalculator->setChecked(true);
    ui->actionStart->setChecked(false);
    editor->setModified(false);
    calculator->setModified(false);
}

QString writeableDir() {
    static QString dir;
    if (!dir.isEmpty())
        return dir;
    QStringList locations = (QStringList()
                             << QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation)
                             << QStandardPaths::standardLocations(QStandardPaths::HomeLocation));
    for (const QString &loc : locations)
        if (QFileInfo::exists(loc)) {
            dir = loc;
            return dir;
        }
    return QString();
}

bool MainWindow::saveFile(const QString &fileName) {
    if (editor->isModified())
        updateExtendedList();
    Ingredients::ingredients = editor->_tmpIngredients;
    bool ret = Ingredients::saveData(fileName);
    if (ret) {
        calculator->updateDisplay();
        editor->setModified(false);
    }
    statusBar()->showMessage(Ingredients::errorString());
    return ret;
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
    for (auto ingredient : newIngr)
        if (!extIngr.contains(ingredient) && !combIngr.contains(ingredient))
            data << ingredient.name() << " = " << ingredient.calories() << '\n';
    file.close();
}

void MainWindow::on_actionMoveUp_triggered() { editor->moveUp(); }

void MainWindow::on_actionMoveDown_triggered() { editor->moveDown(); }

void MainWindow::on_action_export_to_pdf_triggered() {
    auto caloriesWidgets = editor->_tmpIngredients;
    QList<int> kcalList;
    QStringList labelData;
    QStringList lineData;
    auto labelsList = calculator->findChildren<QLabel *>();
    auto linesList = calculator->findChildren<QLineEdit *>();
    for (auto line : linesList)
        if (line->text()!="") {
            lineData.append(line->text());
            labelData.append(labelsList[linesList.indexOf(line)+editor->columns()+6]->text());
        }
    for (auto widget : caloriesWidgets)
        for (auto label : labelData)
            if (widget.name().startsWith(label))
                kcalList.append(widget.calories());

    QString fileName = QFileDialog::getSaveFileName(nullptr, "Export PDF", writeableDir(), "*.pdf");
    if (fileName.isEmpty())
        return;
    if (QFileInfo(fileName).suffix().isEmpty())
        fileName.append(".pdf");
    QPrinter printer(QPrinter::PrinterResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setPaperSize(QPrinter::A4);
    printer.setOutputFileName(fileName);

    QStringList ingrList;
    for (int i=0; i<labelData.count(); i++) {
        QString ingr = labelData[i] + ": " + lineData[i];
        ingrList.append("<span>&#8226; " + ingr + "</span>");
    }
    QStringList instrList;
    for (const QString &line : calculator->instruct->toPlainText().split("\n"))
        instrList.append("<span>&#8226; " + line + "</span>");

    QTextDocument doc;
    QFileInfo fi(fileName);
    doc.setHtml("<p style='text-align: right'> Σύνολο: " + labelsList[1]->text() + "<br/>" + labelsList[5]->text() + "</p>" + "<br/>" \
                + "<b>" + fi.baseName() + "</b>" + "<br/><br/>" + ingrList.join(" γρ.<br/>") + " γρ." \
                + "<br/><br/>" + "Οδηγίες εκτέλεσης:"+ "<br/>" + instrList.join("<br/>"));
    doc.print(&printer);
    ingrList.clear();
}

void MainWindow::saveRecipeFile(QStringList ingrs) {
    QString fileName = QFileDialog::getSaveFileName(this, tr("Αποθήκευση"), writeableDir(),
                                                    QString("Recipies (*.rcp);;Text files (*.txt);;All files (*.*)"));
    if (fileName.isEmpty())
        return;
    QFileInfo fi(fileName);
    if (fi.suffix().isEmpty())
        fileName += ".rcp";
    QSaveFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QFile::Text)) {
        qWarning() << tr("error opening %1").arg(fileName);
        return;
    }
    QTextStream data(&file);
    data.setCodec(QTextCodec::codecForName("UTF-8"));
    data.setGenerateByteOrderMark(true);
    data.setIntegerBase(10);
    for (const QString &ingredient : ingrs)
        data << ingredient << '\n';
    if (data.status() != QTextStream::Ok) {
        qWarning() << tr("error saving %1").arg(fileName);
        return;
    }
    file.commit();
    currentFile = fileName;
    setWindowTitle(QString("%1 - %2%3").arg(QApplication::applicationName())
                   .arg(fi.fileName())
                   .arg(fileName.startsWith(':') ? " [Preset]" : ""));
}

void MainWindow::on_actionSaveRecipe_triggered() {
    if (currentFile.isEmpty() || currentFile.startsWith(':'))
        on_actionSaveRecipeAs_triggered();
    else {
        if (editor->isModified())
            updateExtendedList();
        Ingredients::ingredients = editor->_tmpIngredients;
        QSaveFile file(currentFile);
        if (!file.open(QIODevice::WriteOnly | QFile::Text)) {
            qWarning() << QObject::tr("Σφάλμα ανοίγματος αρχείου: %1").arg(file.errorString());
            return;
        }
        else {
            ingrs.clear();
            auto caloriesWidgets = editor->_tmpIngredients;
            auto masses = calculator->findChildren<QLineEdit *>();
            QList<int> kcalList;
            QStringList labelData;
            QStringList lineData;
            for (auto widget : caloriesWidgets) {
                labelData.append(widget.name());
                kcalList.append(widget.calories());
            }
            for (auto mass : masses)
                lineData.append(mass->text());
            if (lineData.count()!=labelData.count())
                return;
            for (int i=0; i<labelData.count(); i++) {
                QString ingr = labelData[i] + " > " + QString::number(kcalList[i]) + " > " + lineData[i];
                ingrs.append(ingr);
            }
            ingrs.append("#\n" + calculator->instruct->toPlainText());
            QTextStream data(&file);
            data.setCodec(QTextCodec::codecForName("UTF-8"));
            data.setGenerateByteOrderMark(true);
            data.setIntegerBase(10);
            for (const QString &ingredient : ingrs)
                data << ingredient << '\n';
            if (data.status() != QTextStream::Ok) {
                qWarning() << tr("error saving %1").arg(currentFile);
                return;
            }
            file.commit();
            calculator->updateDisplay();
            editor->setModified(false);
            calculator->setModified(false);
            statusBar()->showMessage(Ingredients::errorString());
            ingrs.clear();
        }
    }
}

void MainWindow::on_actionSaveRecipeAs_triggered() {
    if (editor->_tmpIngredients.isEmpty())
        statusBar()->showMessage(tr("Δεν υπάρχει ανοιχτή συνταγή για αποθήκευση"), 3000);
    else {
        ingrs.clear();
        auto caloriesWidgets = editor->_tmpIngredients;
        auto masses = calculator->findChildren<QLineEdit *>();
        QList<int> kcalList;
        QStringList labelData;
        QStringList lineData;
        for (auto widget : caloriesWidgets) {
            labelData.append(widget.name());
            kcalList.append(widget.calories());
        }
        for (auto mass : masses)
            lineData.append(mass->text());
        if (lineData.count()!=labelData.count())
            return;
    
        for (int i=0; i<labelData.count(); i++) {
            QString ingr = labelData[i] + " > " + QString::number(kcalList[i]) + " > " + lineData[i];
            ingrs.append(ingr);
        }
        ingrs.append("#\n" + calculator->instruct->toPlainText());
        saveRecipeFile(ingrs);
        editor->setModified(false);
        calculator->setModified(false);
        ingrs.clear();
    }
}

void MainWindow::on_actionOpenRecipe_triggered() {
    if (editor->isModified() || calculator->isModified()) {
        const QMessageBox::StandardButton ret
            = QMessageBox::warning(this, QApplication::applicationName(),
                                   tr("Υπάρχουν αλλαγές που δεν αποθηκεύτηκαν.\n"),
                                   QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
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
    setWindowTitle(QString("%1 - %2%3").arg(QApplication::applicationName())
                   .arg(fi.fileName())
                   .arg(fileName.startsWith(':') ? " [Preset]" : ""));

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

void MainWindow::on_actionAdaptor_triggered() {
    Adaptor adaptor;
    int ret = adaptor.exec();
    if (ret == QDialog::Rejected)
        return;
    auto lines = calculator->findChildren<QLineEdit *>();
    for (auto line : lines) {
        if (line->text().isNull())
            line->setText("0");
        else {
            int newMass = line->text().toInt() * adaptor.getFrac();
            line->setText(QString::number(newMass));
        }
    }
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
                tr("Program created by Dimitris Psathas<br/><br/>"
                   "Special contributor: Asterios Dimitriou<br/><br/>"
                   "Original idea by Nefeli Vroulli<br/><br/>"
                   "Written in C++, built with the Qt5 toolkit<br/><br/>"
                   "Published under the GNU General Public License v3.0<br/>"
                   "Qt Libraries used under (L)GPLv3<br/><br/>"
                   "&copy; Dimitris Psathas 2020")));
}

void MainWindow::readSettings() {
    QSettings settings;
    bool isMax = settings.value("isMaximized", false).toBool();
    if (isMax)
        showMaximized();
    else {
        const QByteArray geometry = settings.value("geometry", QByteArray()).toByteArray();
        restoreGeometry(geometry);
    }
    const QString f = settings.value("font", QFont()).toString();
    const int s = settings.value("size", 11).toInt();
    const QFont font(f, s);
    QApplication::setFont(font);
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if (editor->isModified() || calculator->isModified()) {
        const QMessageBox::StandardButton ret
            = QMessageBox::warning(this, QApplication::applicationName(),
                                   tr("Υπάρχουν αλλαγές που δεν αποθηκεύτηκαν.\n"),
                                   QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        switch (ret) {
        case QMessageBox::Save:
            on_actionSaveRecipe_triggered();
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
    event->accept();
}