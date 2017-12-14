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
    LOG("BOOM!!");
}

void MainWindow::wclt_timer_event()
{
    if(this->wcltStatus > 0)
    {
        this->on_wsvrRefresh_clicked();
    }
}

void MainWindow::wsvr_set_ui_enabled(bool enable)
{
    this->ui->wsvrGroup->setEnabled(enable);
    this->wsvr_set_ctrl_enabled(this->ui->wsvrEnableCtrl->checkState() == Qt::Checked);
}

void MainWindow::wsvr_set_ctrl_enabled(bool enable)
{
    this->ui->wsvrCtrlPanel->setEnabled(enable);
}

void MainWindow::wsvr_connect()
{
    int ret;
    bool ok;
    string ip;
    int port;

    // Parse ip and port
    ip = this->ui->wsvrIP->text().toStdString();
    port = this->ui->wsvrPort->text().toInt(&ok);
    if(!ok)
    {
        string tmp = "Cannot convert '" + this->ui->wsvrPort->text().toStdString() + "' to server port!";
        QMessageBox qMsg;
        qMsg.setWindowTitle(QString("Error"));
        qMsg.setText(QString(tmp.c_str()));
        qMsg.exec();
        return;
    }

    ret = wclt_connect(&this->wclt, ip.c_str(), port);
    if(ret < 0)
    {
        QMessageBox qMsg;
        qMsg.setWindowTitle(QString("Error"));
        qMsg.setText(QString("Connect Failed!"));
        qMsg.exec();
        return;
    }

    // Enable ui
    this->wsvr_set_ui_enabled(true);

    // Change status
    this->wcltStatus = 1;
    this->ui->wsvrButton->setText("Disconnect");
}

void MainWindow::wsvr_disconnect()
{
    // Disconnect
    wclt_disconnect(this->wclt);

    // Disable ui
    this->wsvr_set_ui_enabled(false);

    // Change status
    this->wcltStatus = 0;
    this->ui->wsvrButton->setText("Connect");
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
            tmp = QString("Error");
            this->ui->wsvrSal->setText(tmp);
            this->ui->wsvrSar->setText(tmp);

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
