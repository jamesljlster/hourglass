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

    void on_wsvrUpdateInterval_valueChanged(double arg1);

    void wclt_timer_event();

    void on_wsvrEnableCtrl_stateChanged(int arg1);

    void on_wsvrSpeedBar_valueChanged(int value);

    void on_wsvrSpeedSpin_valueChanged(int arg1);

    void on_wsvrCtrlPanel_stateChanged(float r, float theta);

    void on_wsvrLock_clicked();

    void on_tsButton_clicked();

    void on_tsUpdateInterval_valueChanged(double arg1);

    void on_tsRefresh_clicked();

    void ts_timer_event();

    void on_tsUpload_clicked();

    void on_tsDownload_clicked();

private:
    Ui::MainWindow *ui;

    int wcltStatus;
    int wcltLocked;
    wclt_t wclt;
    QTimer* wcltTimer;

    int tsStatus;
    trasvc_client_t ts;
    QTimer* tsTimer;

    // Private functions
    void wsvr_connect();
    void wsvr_disconnect();
    void wsvr_lock();
    void wsvr_unlock();
    void wsvr_set_ui_enabled(bool enable);
    void wsvr_set_ctrl_enabled(bool enable);

    void ts_connect();
    void ts_disconnect();
    void ts_set_ui_enabled(bool enable);

};

#endif // MAINWINDOW_H
