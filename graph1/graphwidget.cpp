//
// Serge Spraiter 20150609
//

#include <QtWidgets>

#include "graphwidget.h"

GraphWidget::GraphWidget(QWidget *parent) : QWidget(parent)
{
    shift = 0;
}

void GraphWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    int wid = width() - 1;
    int hei = height() - 1;
    // rect
    QPen pen1(Qt::gray);
    painter.setPen(pen1);
    QBrush brush(Qt::white);
    painter.setBrush(brush);
    painter.drawRect(0, 0, wid, hei);
    // central
    QPen pen2(Qt::lightGray);
    painter.setPen(pen2);
    painter.drawLine(0, hei >> 1, wid, hei >> 1);
    // data
    QPen pen3(Qt::red);
    painter.setPen(pen3);
    painter.translate(shift, height() / 2); // only Y
    painter.drawPolyline(polygon);
    // window
//    painter.setWindow(shift, -0x8000, width(), +0x10000);
//    painter.drawPolyline(polygon);
    painter.end();
}

void GraphWidget::addData(quint16 val)
{
    int wid = width() - 1;
    if (polygon.size() >= wid)
    {
        polygon.removeAt(0);
        shift--;
        if (shift > 0)
            shift = 0;
    }
    int half = height() >> 1;
    int num, y;
    if (val & 0x8000) // twos complement
    {
        num = +(val & 0x7fff) + 1;
        y = +(num * half) / 0x7fff;
    }
    else
    {
        num = val;
        y = -(num * half) / 0x7fff;
    }
    // window
//    int wid = width() - 1;
//    if (polygon.size() >= wid)
//    {
//        polygon.removeAt(0);
//        shift++;
//        if (shift < 0)
//            shift = 0;
//    }
//    int y;
//    if (val & 0x8000) // twos complement
//        y = +(val & 0x7fff) + 1;
//    else
//        y = -val;
    QPoint point;
    point.setX(polygon.count() - shift);
    // window
    //point.setX(polygon.count() + shift);
    point.setY(y);
    polygon.append(point);
    update();
}

void GraphWidget::clear()
{
    polygon.clear();
    update();
}
