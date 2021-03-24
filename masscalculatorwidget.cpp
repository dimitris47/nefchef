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

#include "masscalculatorwidget.h"
#include "ui_masscalculatorwidget.h"
#include "ingredients.h"
#include "masslineedit.h"
#include <QFile>
#include <QGridLayout>
#include <QIntValidator>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QPushButton>
#include <QSizePolicy>
#include <QStatusBar>
#include <QTextCodec>
#include <QTextStream>
#include <QtMath>

int CollectionPage::_columns {3};

MassCalculatorWidget::MassCalculatorWidget(QWidget *parent) :
    CollectionPage(parent),
    ui(new Ui::MassCalculatorWidget)
{
    ui->setupUi(this);
    connect(ui->actionClear, &QAction::triggered, this, &MassCalculatorWidget::clear);
    instruct = new QPlainTextEdit(this);
    instruct->setVisible(false);
}

MassCalculatorWidget::~MassCalculatorWidget() { delete ui; }

void MassCalculatorWidget::calculation() {
    int masssum {0};
    float kcalsum {0};
    for (auto &&lineEdit : lineEdits) {
        int mass = lineEdit->text().toInt();
        masssum += mass;
        kcalsum += lineEdit->calories() * mass / 100.0;
    }
    float percentsum {0};
    if (masssum)
        percentsum = kcalsum*100/masssum;
    ui->kcalcount->setText(QString::number(qRound(kcalsum)) + " kCal");
    ui->masscount->setText(QString::number(masssum) + " g");
    ui->percentcount->setText(QString::number(qRound(percentsum)) + " kCal/100g");
}

void MassCalculatorWidget::clear() {
    for (auto &&lineEdit : lineEdits)
        lineEdit->setText("");
    ui->kcalcount->setText("0 kCal");
    ui->masscount->setText("0 g");
    ui->percentcount->setText("0 kCal/100g");
    lineEdits.at(0)->setFocus();
}

void MassCalculatorWidget::updateDisplay() {
    setUpdatesEnabled(false); // to avoid screen flicker

    auto lastLines = findChildren<QLineEdit *>();
    QStringList lastMasses;
    for (auto &&line : lastLines)
        lastMasses.append(line->text());
    QRegularExpression re("^ing.*$");
    auto caloriesWidgets = findChildren<QWidget *>(re);
    for (auto &&widget : caloriesWidgets)
        widget->close();
    ui->caloriesGridLayout->invalidate();
    lineEdits.clear();

    int rowsPerColumn = qCeil(static_cast<float>(Ingredients::ingredients.size()) / columns());
    for (int i = 0; i < columns(); i++) {
        int column = columns() * i + 1;
        QLabel *label = new QLabel(this);
        label->setObjectName(QString::fromUtf8("ingHeader") + QString::number(i));
        label->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed, QSizePolicy::Label));
        label->setText(tr("γραμμάρια"));
        label->setAlignment(Qt::AlignCenter);
        label->setAttribute(Qt::WA_DeleteOnClose);
        ui->caloriesGridLayout->addWidget(label, 0, column);
        QFrame *vline = new QFrame(this);
        vline->setObjectName(QString::fromUtf8("ingLine") + QString::number(i));
        vline->setFrameShape(QFrame::VLine);
        vline->setFrameShadow(QFrame::Sunken);
        vline->setAttribute(Qt::WA_DeleteOnClose);
        ui->caloriesGridLayout->addWidget(vline, 0, column + 1, rowsPerColumn + 1, 1);
    }

    for (int i = 0; i < Ingredients::ingredients.size(); i++) {
        int column = (i / rowsPerColumn);
        int row = i % rowsPerColumn + 1;
        QLabel *label = new QLabel(this);
        label->setObjectName(QString::fromUtf8("ingLabel") + QString::number(i));
        label->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed, QSizePolicy::Label));
        label->setText(Ingredients::ingredients.at(i).name());
        label->setToolTip(QString("%1 kCal/100g").arg(Ingredients::ingredients.at(i).calories()));
        label->setAttribute(Qt::WA_DeleteOnClose);
        ui->caloriesGridLayout->addWidget(label, row, 0 + column * columns(), 1, 1);

        MassLineEdit *line = new MassLineEdit(Ingredients::ingredients.at(i).calories(), this);
        line->setObjectName(QString::fromUtf8("ingLine") +  QString::number(i));
        line->setAlignment(Qt::AlignCenter);
        line->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed, QSizePolicy::LineEdit));
        line->setValidator(new QIntValidator(0, 100000, this));
        line->setAttribute(Qt::WA_DeleteOnClose);
        ui->caloriesGridLayout->addWidget(line, row, 1 + column * columns(), 1, 1, Qt::AlignHCenter);
        lineEdits.append(line);
        connect(line, &MassLineEdit::textEdited, this, &MassCalculatorWidget::calculation);
        connect(line, &MassLineEdit::textEdited, this, [=]() {
            _modified = true;
        });
    }

    ui->caloriesGridLayout->addWidget(instruct, 0, columns()*2+3, rowsPerColumn + 1, 1);
    instruct->setAttribute(Qt::WA_DeleteOnClose);
    instruct->setPlaceholderText("Οδηγίες εκτέλεσης της συνταγής "
                                 "(στην εξαγωγή σε PDF εισάγονται αυτόματα bullet points σε κάθε χειροκίνητη αλλαγή σειράς)");
    if (columns())
        instruct->setVisible(true);
    connect(instruct, &QPlainTextEdit::textChanged, this, [=]() {
        _modified = true;
    });

    setUpdatesEnabled(true);
    clear();
    if (lineEdits.count() >= lastMasses.count())
        for (int i = 0; i < lastMasses.count(); i++)
            lineEdits.at(i)->setText(lastMasses.at(i));
}

void MassCalculatorWidget::updateMasses(QStringList masses) {
    setUpdatesEnabled(false);

    QRegularExpression re("^ing.*$");
    auto caloriesWidgets = findChildren<QWidget *>(re);
    for (auto &&widget : caloriesWidgets)
        widget->close();
    ui->caloriesGridLayout->invalidate();
    lineEdits.clear();

    int rowsPerColumn = qCeil(static_cast<float>(Ingredients::ingredients.size()) / columns());
    for (int i = 0; i < columns(); i++) {
        int column = columns() * i + 1;
        QLabel *label = new QLabel(this);
        label->setObjectName(QString::fromUtf8("ingHeader") + QString::number(i));
        label->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed, QSizePolicy::Label));
        label->setText(tr("γραμμάρια"));
        label->setAlignment(Qt::AlignCenter);
        label->setAttribute(Qt::WA_DeleteOnClose);
        ui->caloriesGridLayout->addWidget(label, 0, column);
        QFrame *vline = new QFrame(this);
        vline->setObjectName(QString::fromUtf8("ingLine") + QString::number(i));
        vline->setFrameShape(QFrame::VLine);
        vline->setFrameShadow(QFrame::Sunken);
        vline->setAttribute(Qt::WA_DeleteOnClose);
        ui->caloriesGridLayout->addWidget(vline, 0, column + 1, rowsPerColumn + 1, 1);
    }

    for (int i = 0; i < Ingredients::ingredients.size(); i++) {
        int column = (i / rowsPerColumn);
        int row = i % rowsPerColumn + 1;
        QLabel *label = new QLabel(this);
        label->setObjectName(QString::fromUtf8("ingLabel") + QString::number(i));
        label->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed, QSizePolicy::Label));
        label->setText(Ingredients::ingredients.at(i).name());
        label->setToolTip(QString("%1 kCal/100g").arg(Ingredients::ingredients.at(i).calories()));
        label->setAttribute(Qt::WA_DeleteOnClose);
        ui->caloriesGridLayout->addWidget(label, row, 0 + column * columns(), 1, 1);

        MassLineEdit *line = new MassLineEdit(Ingredients::ingredients.at(i).calories(), this);
        line->setObjectName(QString::fromUtf8("ingLine") +  QString::number(i));
        line->setAlignment(Qt::AlignCenter);
        line->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed, QSizePolicy::LineEdit));
        line->setValidator(new QIntValidator(0, 100000, this));
        line->setAttribute(Qt::WA_DeleteOnClose);
        ui->caloriesGridLayout->addWidget(line, row, 1 + column * columns(), 1, 1, Qt::AlignHCenter);
        lineEdits.append(line);
        connect(line, &MassLineEdit::textEdited, this, &MassCalculatorWidget::calculation);
        connect(line, &MassLineEdit::textEdited, this, [=]() {
            _modified = true;
        });
    }

    ui->caloriesGridLayout->addWidget(instruct, 0, columns()*2+3, rowsPerColumn + 1, 1);
    instruct->setAttribute(Qt::WA_DeleteOnClose);
    instruct->setPlaceholderText("Οδηγίες εκτέλεσης της συνταγής "
                                 "(στην εξαγωγή σε PDF εισάγονται αυτόματα bullet points σε κάθε χειροκίνητη αλλαγή σειράς)");
    if (columns())
        instruct->setVisible(true);
    connect(instruct, &QPlainTextEdit::textChanged, this, [=]() {
        _modified = true;
    });

    setUpdatesEnabled(true);
    clear();
    if (lineEdits.count() >= masses.count())
        for (int i=0; i<masses.count(); i++)
            lineEdits.at(i)->setText(masses.at(i));
}

void MassCalculatorWidget::addIngr(QString name) {
    setUpdatesEnabled(false);

    auto lastLines = findChildren<QLineEdit *>();
    QStringList lastMasses;
    for (auto &&line : lastLines)
        lastMasses.append(line->text());
    QRegularExpression re("^ing.*$");
    auto caloriesWidgets = findChildren<QWidget *>(re);
    for (auto &&widget : caloriesWidgets)
        widget->close();
    ui->caloriesGridLayout->invalidate();
    lineEdits.clear();

    int rowsPerColumn = qCeil(static_cast<float>(Ingredients::ingredients.size()) / columns());
    for (int i = 0; i < columns(); i++) {
        int column = columns() * i + 1;
        QLabel *label = new QLabel(this);
        label->setObjectName(QString::fromUtf8("ingHeader") + QString::number(i));
        label->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed, QSizePolicy::Label));
        label->setText(tr("γραμμάρια"));
        label->setAlignment(Qt::AlignCenter);
        label->setAttribute(Qt::WA_DeleteOnClose);
        ui->caloriesGridLayout->addWidget(label, 0, column);
        QFrame *vline = new QFrame(this);
        vline->setObjectName(QString::fromUtf8("ingLine") + QString::number(i));
        vline->setFrameShape(QFrame::VLine);
        vline->setFrameShadow(QFrame::Sunken);
        vline->setAttribute(Qt::WA_DeleteOnClose);
        ui->caloriesGridLayout->addWidget(vline, 0, column + 1, rowsPerColumn + 1, 1);
    }

    for (int i = 0; i < Ingredients::ingredients.size(); i++) {
        int column = (i / rowsPerColumn);
        int row = i % rowsPerColumn + 1;
        QLabel *label = new QLabel(this);
        label->setObjectName(QString::fromUtf8("ingLabel") + QString::number(i));
        label->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed, QSizePolicy::Label));
        label->setText(Ingredients::ingredients.at(i).name());
        label->setToolTip(QString("%1 kCal/100g").arg(Ingredients::ingredients.at(i).calories()));
        label->setAttribute(Qt::WA_DeleteOnClose);
        ui->caloriesGridLayout->addWidget(label, row, 0 + column * columns(), 1, 1);

        MassLineEdit *line = new MassLineEdit(Ingredients::ingredients.at(i).calories(), this);
        line->setObjectName(QString::fromUtf8("ingLine") +  QString::number(i));
        line->setAlignment(Qt::AlignCenter);
        line->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed, QSizePolicy::LineEdit));
        line->setValidator(new QIntValidator(0, 100000, this));
        line->setAttribute(Qt::WA_DeleteOnClose);
        ui->caloriesGridLayout->addWidget(line, row, 1 + column * columns(), 1, 1, Qt::AlignHCenter);
        lineEdits.append(line);
        connect(line, &MassLineEdit::textEdited, this, &MassCalculatorWidget::calculation);
        connect(line, &MassLineEdit::textEdited, this, [=]() {
            _modified = true;
        });
    }

    QList<QLabel* > labelList = findChildren<QLabel *>();
    labelList.at(labelList.count()-1)->setText(name);

    ui->caloriesGridLayout->addWidget(instruct, 0, columns()*2+3, rowsPerColumn + 1, 1);
    instruct->setAttribute(Qt::WA_DeleteOnClose);
    instruct->setPlaceholderText("Οδηγίες εκτέλεσης της συνταγής "
                                 "(στην εξαγωγή σε PDF εισάγονται αυτόματα bullet points σε κάθε χειροκίνητη αλλαγή σειράς)");
    if (columns())
        instruct->setVisible(true);
    connect(instruct, &QPlainTextEdit::textChanged, this, [=]() {
        _modified = true;
    });

    setUpdatesEnabled(true);
    clear();
    if (lineEdits.count() >= lastMasses.count())
        for (int i=0; i<lastMasses.count(); i++)
            lineEdits.at(i)->setText(lastMasses.at(i));
}
