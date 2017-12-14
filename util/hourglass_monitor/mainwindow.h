#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>

#include <wclt.h>
#include <trasvc.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_wsvrButton_clicked();
    void on_wsvrRefresh_clicked();
    void on_wsvrTimer_timeout();

    void on_wsvrUpdateInterval_valueChanged(double arg1);

private:
    Ui::MainWindow *ui;

    int wcltStatus;
    wclt_t wclt;
    QTimer* wcltTimer;

    int tsStatus;
    trasvc_client_t ts;
    QTimer* tsTimer;
};

#endif // MAINWINDOW_H
