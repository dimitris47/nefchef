#ifndef STARTPAGE_H
#define STARTPAGE_H

#include <QWidget>

namespace Ui { class StartPage; }

class StartPage : public QWidget {
    Q_OBJECT

public:
    explicit StartPage(QWidget *parent = nullptr);
    ~StartPage();

signals:
    void create();
    void help();
    void info();
    void open();

private slots:
    void on_startCreate_clicked();
    void on_startHelp_clicked();
    void on_startInfo_clicked();
    void on_startOpen_clicked();

private:
    Ui::StartPage *ui;
};

#endif // STARTPAGE_H
