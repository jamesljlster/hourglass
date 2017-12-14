#include <QPainter>

#include "qctrlpanel.h"

QCtrlPanel::QCtrlPanel(QWidget *parent) : QWidget(parent)
{
    // Initial values
    this->x = 0;
    this->y = 0;
}

void QCtrlPanel::paintEvent(QPaintEvent *paintEvent)
{
    // Find drawing information
    int width = this->width();
    int height = this->height();
    int radius = (width > height) ? height : width;
    radius /= 2;

    int xShift = this->x + width / 2;
    int yShift = this->y + height / 2;

    // Start drawing
    QPainter painter(this);

    // Draw background
    QBrush bgColor(Qt::white, Qt::SolidPattern);
    painter.setBrush(bgColor);
    painter.drawRect(0, 0, width, height);

    // Draw border cycle
    QPen fgColor(Qt::black);
    painter.drawEllipse(QPoint(width / 2, height / 2), radius, radius);

    // Draw control point
    QBrush ctrlColor(Qt::black, Qt::SolidPattern);
    painter.setBrush(ctrlColor);
    painter.drawEllipse(QPoint(xShift, yShift), radius / 5, radius / 5);
}
