#ifndef QCTRLPANEL_H
#define QCTRLPANEL_H

#include <QWidget>

class QCtrlPanel : public QWidget
{
    Q_OBJECT
public:
    explicit QCtrlPanel(QWidget *parent = nullptr);

    void reset();

    float get_r();
    float get_theta();

signals:
    void stateChanged(float r, float theta);

public slots:

protected:
    void paintEvent(QPaintEvent *paintEvent);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);

private:
    float r;
    float theta;

};

#endif // QCTRLPANEL_H
