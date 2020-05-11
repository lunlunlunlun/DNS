#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <WinSock2.h>
#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void displayed();
    void RecvDnsPack(SOCKET* sock);
    int SendDomainName(SOCKET* sock);
    bool is_valid_DN();

private slots:
    void on_pushButton_clicked();


private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
