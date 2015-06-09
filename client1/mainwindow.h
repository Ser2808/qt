//
// Serge Spraiter 20150608
//

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNetwork>

#include "clientthread.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_pushButton_Connect_clicked();
    void on_pushButton_Browse_clicked();
    void on_pushButton_Send_clicked();
    void on_disconnected();
    void on_connected();
    void on_error(QAbstractSocket::SocketError socketError);
    void on_finished();
    void on_updateStatusBar(const QString &str);

private:
    Ui::MainWindow *ui;
    quint32 interval;
    QTcpSocket *tcpSocket;
    quint64 posInFile;  // position in File
    quint64 posWas;  // position was (to calculate bytes sent)
    quint64 fileSize;
    QFile *file;
    ClientThread *clientThread;
    int stopThread;    // stop thread flag
};

#endif // MAINWINDOW_H
