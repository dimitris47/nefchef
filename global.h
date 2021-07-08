#ifndef GLOBAL_H
#define GLOBAL_H

#include <QString>

const QString APPNAME("NefChef");
const QString VERSION("2.9.1");
const QString CONTRIBUTORS("Dimitris Psathas, Asterios Dimitriou");
const QString br("<br/>");
const QString plh("Οδηγίες εκτέλεσης της συνταγής "
                  "(στην εξαγωγή σε PDF εισάγονται αυτόματα bullet points σε κάθε χειροκίνητη αλλαγή σειράς)");
const QString APPINFO("Program created by Dimitris Psathas" + br.repeated(2) +
                      "Special contributor: Asterios Dimitriou" + br.repeated(2) +
                      "Original idea by Nefeli Vroulli" + br.repeated(2) +
                      "Written in C++, built against Qt 5.15.2" + br.repeated(2) +
                      "Published under the GNU General Public License v3.0" + br +
                      "Qt libraries used under (L)GPL3" + br +
                      "Application icon by <a href='https://icons8.com'>icons8.com</a>" + br +
                      "In-app icons by the Ubuntu Yaru Theme Community" + br.repeated(2) +
                      "&copy; 2020-2021 Dimitris Psathas");

#endif // GLOBAL_H
