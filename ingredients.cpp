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

    QString errorString() { return _errorString; }

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

    bool loadList(const QStringList &list) {
        ingredients.clear();
        for (auto &&line : list)
            if (!insertString(line))
                return false;
        return ingredients.size();
    }
}
