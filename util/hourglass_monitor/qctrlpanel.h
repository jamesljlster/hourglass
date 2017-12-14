#ifndef QCTRLPANEL_H
#define QCTRLPANEL_H

#include <QWidget>

class QCtrlPanel : public QWidget
{
    Q_OBJECT
public:
    explicit QCtrlPanel(QWidget *parent = nullptr);

    float get_r();
    float get_theta();

signals:

public slots:

protected:
    void paintEvent(QPaintEvent *paintEvent);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);

private:
    int x;
    int y;

};

#endif // QCTRLPANEL_H
