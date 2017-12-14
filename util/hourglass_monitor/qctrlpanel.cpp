#include <QPainter>
#include <QMouseEvent>
#include <cmath>

#include "qctrlpanel.h"

#define DEBUG
#include "debug.hpp"

#define LINE_WIDTH 3
#define CTRL_PART 3

using namespace std;

QCtrlPanel::QCtrlPanel(QWidget *parent) : QWidget(parent)
{
    // Setup widget
    this->setMouseTracking(false);

    // Initial values
    this->r = 0;
    this->theta = 0;
}

void QCtrlPanel::mouseMoveEvent(QMouseEvent* event)
{
    int width = this->width();
    int height = this->height();
    int radius = ((width > height) ? height : width) * (CTRL_PART - 1) / (2 * CTRL_PART);

    // Set position
    int xTmp = event->x() - this->width() / 2;
    int yTmp = -event->y() + this->height() / 2;

    // Convert coordinate
    this->r = sqrt(xTmp * xTmp + yTmp * yTmp);
    this->theta = -(atan((float)yTmp / (float)xTmp) - M_PI_2);
    if(xTmp < 0)
    {
        this->theta = this->theta - M_PI;
    }

    // Normalize
    if(this->r > radius)
    {
        this->r = 1;
    }
    else
    {
        this->r = this->r / (float)radius;
    }

    LOG("(x, y) = (%d, %d)", xTmp, yTmp);
    LOG("theta: %f\n", this->theta / M_PI * 180);

    this->repaint();
}

void QCtrlPanel::mouseReleaseEvent(QMouseEvent* event)
{
    // Reset position
    this->r = 0;
    this->theta = 0;

    this->repaint();
}

void QCtrlPanel::paintEvent(QPaintEvent *paintEvent)
{
    // Find drawing information
    int width = this->width();
    int height = this->height();
    int drawRadius = ((width > height) ? height : width) / 2;
    int radius = ((width > height) ? height : width) * (CTRL_PART - 1) / (2 * CTRL_PART);

    int xShift = (this->r * radius) * sin(this->theta) + width / 2;
    int yShift = -((this->r * radius) * cos(this->theta) - height / 2);

    // Start drawing
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Draw background
    QBrush bgColor(Qt::white, Qt::SolidPattern);
    painter.setBrush(bgColor);
    painter.drawRect(0, 0, width, height);

    // Draw border cycle
    QPen fgColor(Qt::black, LINE_WIDTH);
    painter.setPen(fgColor);
    painter.drawEllipse(QPoint(width / 2, height / 2), drawRadius - LINE_WIDTH, drawRadius - LINE_WIDTH);

    // Draw control point
    QBrush ctrlColor(Qt::black, Qt::SolidPattern);
    painter.setBrush(ctrlColor);
    painter.drawEllipse(QPoint(xShift, yShift), drawRadius / CTRL_PART, drawRadius / CTRL_PART);
}
