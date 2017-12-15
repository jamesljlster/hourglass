#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>

#include <cstdio>
#include <cstring>
#include <string>

#define DEBUG
#include "debug.hpp"

using namespace std;

void MainWindow::ts_connect()
{
    int ret;
    bool ok;
    string ip;
    int port;

    // Parse ip and port
    ip = this->ui->tsIP->text().toStdString();
    port = this->ui->tsPort->text().toInt(&ok);
    if(!ok)
    {
        string tmp = "Cannot convert '" + this->ui->tsIP->text().toStdString() + "' to server port!";
        QMessageBox qMsg;
        qMsg.setWindowTitle(QString("Error"));
        qMsg.setText(QString(tmp.c_str()));
        qMsg.exec();
        return;
    }

    ret = trasvc_client_connect(&this->ts, ip.c_str(), port);
    if(ret < 0)
    {
        QMessageBox qMsg;
        qMsg.setWindowTitle(QString("Error"));
        qMsg.setText(QString("Connect Failed!"));
        qMsg.exec();
        return;
    }

    // Enable ui
    this->ts_set_ui_enabled(true);

    // Change status
    this->tsStatus = 1;
    this->ui->tsButton->setText("Disconnect");
}

void MainWindow::ts_disconnect()
{
    // Disconnect
    trasvc_client_disconnect(this->ts);

    // Disable ui
    this->ts_set_ui_enabled(false);

    // Change status
    this->tsStatus = 0;
    this->ui->tsButton->setText("Connect");
}

void MainWindow::ts_set_ui_enabled(bool enable)
{
    this->ui->tsGroup->setEnabled(enable);
}

void MainWindow::ts_timer_event()
{
    if(this->tsStatus > 0)
    {
        this->on_tsRefresh_clicked();
    }
}
