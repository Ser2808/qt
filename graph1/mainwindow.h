//
// Serge Spraiter 20150609
//

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNetwork>

#include "graphwidget.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    GraphWidget *graph;
    QTcpServer *tcpServer;
    QTcpSocket *tcpSocket;
    quint32 adr; // byte address
    QString getDataString(quint16 val);

private slots:
    void on_pushButton_Listen_clicked();
    void on_newConnection();
    void on_disconnected();
    void on_readyRead();
};

#endif // MAINWINDOW_H
