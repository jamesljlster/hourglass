#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>

#include <cstdio>
#include <cstring>
#include <string>

#define DEBUG
#include "debug.hpp"

using namespace std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Set initial status
    this->wcltStatus = 0;
    this->tsStatus = 0;
    this->wsvr_set_ui_enabled(false);

    this->ui->wsvrSpeedSpin->setValue(this->ui->wsvrSpeedBar->value());

    // Initial timer and event
    this->wcltTimer = new QTimer();
    connect(this->wcltTimer, SIGNAL(timeout()), this, SLOT(wclt_timer_event()));
    this->wcltTimer->start(this->ui->wsvrUpdateInterval->value() * 1000);
}

MainWindow::~MainWindow()
{
    delete ui;

    // Disconnect service
    if(this->wcltStatus > 0)
    {
        wclt_disconnect(this->wclt);
    }

    if(this->tsStatus > 0)
    {
        trasvc_client_disconnect(this->ts);
    }

    // Cleanup
    delete this->wcltTimer;
}

void MainWindow::on_wsvrCtrlPanel_stateChanged(float r, float theta)
{
    int sal, sar;
    int speedMax = this->ui->wsvrSpeedBar->value() * 255 / 100;

    // Conversion
    if(cos(theta) >= 0)
    {
        sal = (speedMax * r) * (cos(theta) + sin(theta)) + 255.0;
        sar = (speedMax * r) * (cos(theta) - sin(theta)) + 255.0;
    }
    else
    {
        sal = (speedMax * r) * (cos(theta) - sin(theta)) + 255.0;
        sar = (speedMax * r) * (cos(theta) + sin(theta)) + 255.0;
    }

    // Checking
    if(sal > 510)
    {
        sal = 510;
    }
    else if(sal < 0)
    {
        sal = 0;
    }

    if(sar > 510)
    {
        sar = 510;
    }
    else if(sar < 0)
    {
        sar = 0;
    }

    LOG("sal = %d, sar = %d", sal, sar);

    // Control wheel
    int ret = wclt_control(this->wclt, sal, sar);
    if(ret != 0)
    {
        LOG("wclt_control() failed with error: %d", ret);
        QMessageBox qMsg;
        qMsg.setWindowTitle(QString("Error"));
        qMsg.setText(QString("Cannot connect to wheel server!"));
        qMsg.exec();

        this->wsvr_disconnect();
    }
}

void MainWindow::on_wsvrButton_clicked()
{
    if(this->wcltStatus > 0)
    {
        this->wsvr_disconnect();
    }
    else
    {
        this->wsvr_connect();
    }
}

void MainWindow::on_wsvrRefresh_clicked()
{
    if(this->wcltStatus > 0)
    {
        int sal, sar;
        QString tmp;
        int ret = wclt_get_speed(this->wclt, &sal, &sar);
        if(ret == 0)
        {
            tmp = QString::number(sal);
            this->ui->wsvrSal->setText(tmp);

            tmp = QString::number(sar);
            this->ui->wsvrSar->setText(tmp);
        }
        else
        {
            LOG("wclt_get_speed() failed with error: %d", ret);
            QMessageBox qMsg;
            qMsg.setWindowTitle(QString("Error"));
            qMsg.setText(QString("Cannot connect to wheel server!"));
            qMsg.exec();

            this->wsvr_disconnect();
        }
    }
    else
    {
        QMessageBox qMsg;
        qMsg.setWindowTitle(QString("Error"));
        qMsg.setText(QString("Server not connected!"));
        qMsg.exec();
    }
}

void MainWindow::on_wsvrUpdateInterval_valueChanged(double arg1)
{
    this->wcltTimer->start(arg1 * 1000);
}

void MainWindow::on_wsvrEnableCtrl_stateChanged(int arg1)
{
    if(arg1 == Qt::Checked)
    {
        this->wsvr_set_ctrl_enabled(true);
    }
    else
    {
        this->wsvr_set_ctrl_enabled(false);
    }
}

void MainWindow::on_wsvrSpeedBar_valueChanged(int value)
{
    this->ui->wsvrSpeedSpin->setValue(value);
}

void MainWindow::on_wsvrSpeedSpin_valueChanged(int arg1)
{
    this->ui->wsvrSpeedBar->setValue(arg1);
}

void MainWindow::on_wsvrLock_clicked()
{
    if(this->wcltLocked > 0)
    {
        this->wsvr_unlock();
    }
    else
    {
        this->wsvr_lock();
    }
}
