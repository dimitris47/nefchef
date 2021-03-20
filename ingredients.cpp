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

#include "ingredients.h"
#include <QFile>
#include <QSaveFile>
#include <QTextCodec>
#include <QTextStream>

namespace Ingredients {

QList<Ingredient> ingredients {};
QString _errorString {QLatin1String("")};

static bool insertString(const QString & line) {
    if (line.startsWith('#') || line.isEmpty())
        return true;
    QStringList pair = line.split('=');
    if (pair.size() != 2) {
        _errorString = QObject::tr("Άκυρη γραμμή: '%1'").arg(line);
        return false;
    }
    bool ok;
    int calories = pair.last().toInt(&ok);
    if (!ok) {
        _errorString = QObject::tr("Οι θερμίδες πρέπει να είναι ακέραιος αριθμός");
        return false;
    }
    ingredients << Ingredient(pair.first().trimmed(), calories);
    return true;
}

bool loadData(const QString &fileName) {
    QFile data(fileName);
    if (!data.open(QIODevice::ReadOnly | QIODevice::Text)) {
        _errorString = QObject::tr("Σφάλμα στο άνοιγμα του αρχείου: %1").arg(data.errorString());
        return false;
    }
    ingredients.clear();
    _errorString = QObject::tr("Δεν υπάρχουν δεδομένα");
    QTextStream reader(&data);
    reader.setCodec(QTextCodec::codecForName("UTF-8"));
    while (!reader.atEnd()) {
        QString line = reader.readLine();
        if (!insertString(line))
            return false;
    }
    return ingredients.size();
}

bool loadList(const QStringList &list) {
    ingredients.clear();
    _errorString = QObject::tr("Δεν υπάρχουν δεδομένα");
    for (auto &&line : list)
        if (!insertString(line))
            return false;
    return ingredients.size();
}

bool saveData(const QString &fileName) {
    _errorString=QString();
    QSaveFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QFile::Text)) {
        _errorString = QObject::tr("Σφάλμα ανοίγματος αρχείου: %1").arg(file.errorString());
        return false;
    }
    QTextStream data(&file);
    data.setCodec(QTextCodec::codecForName("UTF-8"));
    data.setGenerateByteOrderMark(true);
    data.setIntegerBase(10);
    data << preamble;
    for (auto &&ingredient : ingredients)
        data << ingredient.name() << " = " << ingredient.calories() << '\n';

    // enum QTextStream::Status -- QTextStream::Ok = 0
    if (data.status() != QTextStream::Ok)
        _errorString = QObject::tr("Σφάλμα στην αποθήκευση του αρχείου %1").arg(fileName);
    _errorString = QObject::tr("Επιτυχής αποθήκευση του αρχείου %1"). arg(fileName);
    return file.commit();
}

QString errorString() {
    return _errorString;
}

const QString preamble = QT_TR_NOOP("# Γραμμές που αρχίζουν με # είναι σχόλια\n"
                                    "# Οι άδειες γραμμές αγνοούνται\n"
                                    "# Η γραμμή πρέπει να έχει μορφή όπως\n"
                                    "#   Συστατικό = θερμίδες ανά 100 γραμμάρια\n"
                                    "#   (string)  = (number)\n"
                                    "# Το διάστημα γύρω από το ίσον είναι προαιρετικό\n"
                                    "# Τα ακόλουθα είναι ισοδύναμα\n"
                                    "#   Ingredient = 100\n"
                                    "#   Ingredient= 100\n"
                                    "#   Ingredient =100\n"
                                    "#   Ingredient=100\n"
                                    "# Οτιδήποτε άλλο θεωρείται λάθος\n\n"
                                    );
}
