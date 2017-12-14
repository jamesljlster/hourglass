#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

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

private:
    Ui::MainWindow *ui;

    int wcltStatus;
    wclt_t wclt;

    int tsStatus;
    trasvc_client_t ts;
};

#endif // MAINWINDOW_H
