//
// Serge Spraiter 20150609
//

#ifndef GRAPHWIDGET_H
#define GRAPHWIDGET_H

#include <QWidget>

class GraphWidget : public QWidget
{
    Q_OBJECT
public:
    explicit GraphWidget(QWidget *parent = 0);
    void addData(quint16 val);
    void clear();

private:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    QPolygon polygon;
    int shift;

signals:
public slots:
};

#endif // GRAPHWIDGET_H
