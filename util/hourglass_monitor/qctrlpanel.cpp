#include <QPainter>
#include <QMouseEvent>

#include "qctrlpanel.h"

#define LINE_WIDTH 3

QCtrlPanel::QCtrlPanel(QWidget *parent) : QWidget(parent)
{
    // Setup widget
    this->setMouseTracking(false);

    // Initial values
    this->x = 0;
    this->y = 0;
}

void QCtrlPanel::mouseMoveEvent(QMouseEvent* event)
{
    // Set position
    this->x = event->x() - this->width() / 2;
    this->y = -event->y() + this->height() / 2;

    this->repaint();
}

void QCtrlPanel::mouseReleaseEvent(QMouseEvent* event)
{
    // Reset position
    this->x = 0;
    this->y = 0;

    this->repaint();
}

void QCtrlPanel::paintEvent(QPaintEvent *paintEvent)
{
    // Find drawing information
    int width = this->width();
    int height = this->height();
    int radius = (width > height) ? height : width;
    radius /= 2;

    int xShift = this->x + width / 2;
    int yShift = -(this->y - height / 2);

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
    painter.drawEllipse(QPoint(width / 2, height / 2), radius - LINE_WIDTH, radius - LINE_WIDTH);

    // Draw control point
    QBrush ctrlColor(Qt::black, Qt::SolidPattern);
    painter.setBrush(ctrlColor);
    painter.drawEllipse(QPoint(xShift, yShift), radius / 3, radius / 3);
}
