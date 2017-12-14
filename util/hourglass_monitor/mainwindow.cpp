#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>

#include <cstdio>
#include <cstring>
#include <string>

using namespace std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Set initial status
    this->wcltStatus = 0;
    this->tsStatus = 0;
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
}

void MainWindow::on_wsvrButton_clicked()
{
    if(this->wcltStatus > 0)
    {
        // Disconnect
        wclt_disconnect(this->wclt);

        // Change status
        this->wcltStatus = 0;
        this->ui->wsvrButton->setText("Connect");
    }
    else
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
            qMsg.setText(QString(tmp.c_str()));
            qMsg.exec();
            return;
        }

        ret = wclt_connect(&this->wclt, ip.c_str(), port);
        if(ret < 0)
        {
            QMessageBox qMsg;
            qMsg.setText(QString("Connect Failed!"));
            qMsg.exec();
            return;
        }

        // Change status
        this->wcltStatus = 1;
        this->ui->wsvrButton->setText("Disconnect");
    }
}
