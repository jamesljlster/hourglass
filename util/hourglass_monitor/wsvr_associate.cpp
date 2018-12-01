#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>

#include <cstdio>
#include <cstring>
#include <string>

#define DEBUG
#include "debug.hpp"

using namespace std;

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

    // Initial lock status
    this->wcltLocked = 0;
    this->ui->wsvrLock->setText(QString("Lock Device"));
}

void MainWindow::wsvr_disconnect()
{
    // Disconnect
    wclt_disconnect(this->wclt);
    if(this->wcltLocked)
    {
        this->wsvr_unlock();
    }

    // Disable ui
    this->wsvr_set_ui_enabled(false);

    // Change status
    this->wcltStatus = 0;
    this->ui->wsvrButton->setText("Connect");
}

void MainWindow::wsvr_lock()
{
    int ret = wclt_lock(this->wclt);
    if(ret != 0)
    {
        QMessageBox qMsg;
        qMsg.setWindowTitle(QString("Error"));
        qMsg.setText(QString("Failed to lock device!"));
        qMsg.exec();
        return;
    }

    // Change status
    this->wcltLocked = 1;
    this->ui->wsvrLock->setText("Unlock Device");
}

void MainWindow::wsvr_unlock()
{
    int ret = wclt_unlock(this->wclt);
    if(ret != 0)
    {
        QMessageBox qMsg;
        qMsg.setWindowTitle(QString("Error"));
        qMsg.setText(QString("Failed to unlock device!"));
        qMsg.exec();
        return;
    }

    // Change status
    this->wcltLocked = 0;
    this->ui->wsvrLock->setText("Lock Device");
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
    this->ui->wsvrCtrlPanel->reset();
}
