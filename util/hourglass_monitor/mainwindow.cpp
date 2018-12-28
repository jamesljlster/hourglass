#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QFileDialog>

#include <cstdio>
#include <cstring>
#include <string>
#include <cmath>

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
    this->ts_set_ui_enabled(false);

    this->ui->wsvrSpeedSpin->setValue(this->ui->wsvrSpeedBar->value());

    // Initial timer and event
    this->wcltTimer = new QTimer();
    connect(this->wcltTimer, SIGNAL(timeout()), this, SLOT(wclt_timer_event()));
    this->wcltTimer->start(this->ui->wsvrUpdateInterval->value() * 1000);

    this->tsTimer = new QTimer();
    connect(this->tsTimer, SIGNAL(timeout()), this, SLOT(ts_timer_event()));
    this->tsTimer->start(this->ui->tsUpdateInterval->value() * 1000);
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

void MainWindow::on_tsButton_clicked()
{
    if(this->tsStatus > 0)
    {
        this->ts_disconnect();
    }
    else
    {
        this->ts_connect();
    }
}

void MainWindow::on_tsUpdateInterval_valueChanged(double arg1)
{
    this->tsTimer->start(arg1 * 1000);
}

void MainWindow::on_tsRefresh_clicked()
{
    int flag, ret;
    float mse;
    QMessageBox qMsg;

    if(this->tsStatus > 0)
    {
        // Get status
        ret = trasvc_client_get_status(this->ts, &flag);
        if(ret != 0)
        {
            LOG("trasvc_client_get_status() failed with error: %d", ret);
            goto ERR;
        }

        // Get mse
        ret = trasvc_client_get_mse(this->ts, &mse);
        if(ret != 0)
        {
            LOG("trasvc_client_get_mse() failed with error: %d", ret);
            goto ERR;
        }

        // Set ui text
        this->ui->tsActive->setText(QString(to_string((flag & TRASVC_ACTIVE) > 0).c_str()));
        this->ui->tsMgrEmpty->setText(QString(to_string((flag & TRASVC_MGRDATA_EMPTY) > 0).c_str()));
        this->ui->tsMgrFull->setText(QString(to_string((flag & TRASVC_MGRDATA_FULL) > 0).c_str()));
        this->ui->tsTraEmpty->setText(QString(to_string((flag & TRASVC_TRADATA_EMPTY) > 0).c_str()));
        this->ui->tsTraFull->setText(QString(to_string((flag & TRASVC_TRADATA_FULL) > 0).c_str()));
        this->ui->tsMse->setText(QString(to_string(mse).c_str()));
    }
    else
    {
        QMessageBox qMsg;
        qMsg.setWindowTitle(QString("Error"));
        qMsg.setText(QString("Server not connected!"));
        qMsg.exec();
    }

    return;

ERR:
    qMsg.setWindowTitle(QString("Error"));
    qMsg.setText(QString("Cannot connect to LSTM training server!"));
    qMsg.exec();
    this->ts_disconnect();
}

void MainWindow::on_tsUpload_clicked()
{
    // Get model path
    QString fPath = QFileDialog::getOpenFileName(this, QString("Select LSTM Model to Upload"));
    if(fPath.isEmpty())
    {
        return;
    }

    // Upload lstm model
    int ret = trasvc_client_model_upload(this->ts, fPath.toStdString().c_str());
    if(ret != TRASVC_NO_ERROR)
    {
        LOG("trasvc_client_upload_model() failed with error: %d", ret);
        QMessageBox qMsg;
        qMsg.setWindowTitle(QString("Error"));
        qMsg.setText(QString("Failed to upload target LSTM model!"));
        qMsg.exec();
    }
}

void MainWindow::on_tsDownload_clicked()
{
    // Get save model path
    QString fPath = QFileDialog::getSaveFileName(this, QString("Select LSTM Model Save Path"));
    if(fPath.isEmpty())
    {
        return;
    }

    // Download lstm model
    lstm_t tmpLstm = NULL;
    int ret = trasvc_client_model_download(this->ts, &tmpLstm);
    if(ret != TRASVC_NO_ERROR)
    {
        LOG("trasvc_client_upload_model() failed with error: %d", ret);
        QMessageBox qMsg;
        qMsg.setWindowTitle(QString("Error"));
        qMsg.setText(QString("Failed to download LSTM model!"));
        qMsg.exec();
    }
    else
    {
        // Export lstm model
        ret = lstm_export(tmpLstm, fPath.toStdString().c_str());
        if(ret != LSTM_NO_ERROR)
        {
            LOG("lstm_export() failed with error: %d", ret);
            QMessageBox qMsg;
            qMsg.setWindowTitle(QString("Error"));
            qMsg.setText(QString("Failed to save LSTM model!"));
            qMsg.exec();
        }

        lstm_delete(tmpLstm);
    }
}
