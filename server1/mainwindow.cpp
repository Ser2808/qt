//
// Serge Spraiter 20150608
//

#include <QtWidgets>

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow), tcpSocket(0)
{
    ui->setupUi(this);
    tcpServer = new QTcpServer(this);
    tcpServer->setMaxPendingConnections(1);
    connect(tcpServer, SIGNAL(newConnection()), this, SLOT(on_newConnection()));
}

MainWindow::~MainWindow()
{
    if (tcpSocket)
    {
        tcpSocket->close();
        delete tcpSocket;
    }
    delete tcpServer;
    delete ui;
}

void MainWindow::on_pushButton_Listen_clicked()
{
    if (ui->pushButton_Listen->text() == "Listen")
    {
        if (ui->lineEditIP->text().indexOf(':') < 0)
        {
            QMessageBox::critical(this, "Server1", "Missing ':' in IP:Port");
            return;
        }
        QStringList split = ui->lineEditIP->text().split( ":" );
        bool ok;
        quint32 port = split[1].toUInt(&ok);
        if (!ok || (port > 65535))
        {
            QMessageBox::critical(this, "Server1", "Invalid Port, must be 0..65535");
            return;
        }
        QHostAddress adr;
        if (!adr.setAddress(split[0]))
        {
            QMessageBox::critical(this, "Server1", "Invalid IP Address");
            return;
        }
        ui->textEdit->append(QString("Listening on %1:%2...").arg(adr.toString(), QString("%1").arg(port)));
        ui->textEdit->verticalScrollBar()->setValue(ui->textEdit->verticalScrollBar()->maximum());
        if (!tcpServer->listen(adr, port))
        {
            QMessageBox::critical(this, "Server1", "Unable to start listening");
            return;
        }
        ui->labelIP->setEnabled(false);
        ui->lineEditIP->setEnabled(false);
        ui->pushButton_Listen->setText("Stop listening");
    }
    else
    {
        if (tcpServer && tcpServer->isListening())
        {
            if (tcpSocket)
                tcpSocket->disconnectFromHost();
            tcpServer->close();
            ui->textEdit->append("Stops listening");
            ui->textEdit->verticalScrollBar()->setValue(ui->textEdit->verticalScrollBar()->maximum());
        }
        ui->labelIP->setEnabled(true);
        ui->lineEditIP->setEnabled(true);
        ui->pushButton_Listen->setText("Listen");
    }
}

void MainWindow::on_newConnection()
{
    if (tcpSocket)
    {
        tcpSocket->disconnectFromHost();   // only 1 connection
        delete tcpSocket;
        tcpSocket = 0;
    }
    tcpSocket = tcpServer->nextPendingConnection();
    connect(tcpSocket, SIGNAL(disconnected()), this, SLOT(on_disconnected()));
    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(on_readyRead()));
    ui->textEdit->append("Client connected");
    ui->textEdit->verticalScrollBar()->setValue(ui->textEdit->verticalScrollBar()->maximum());
}

void MainWindow::on_disconnected()
{
    ui->textEdit->append("Client disconnected");
    ui->textEdit->verticalScrollBar()->setValue(ui->textEdit->verticalScrollBar()->maximum());
}

void MainWindow::on_readyRead()
{
    QByteArray byteArray(0);
    while (tcpSocket->bytesAvailable())
    {
        byteArray.append(tcpSocket->readAll());
        QString str;
        uchar c0;
        for (int i0 = 0; i0 < byteArray.size() ; i0++)
        {
            c0 = byteArray[i0];
            if (((c0 >= 0x20) && (c0 <= 126)) || (c0 == '\n'))
                str.append(c0);
            else
                if (c0 != '\r') // skip \r
                    str.append(QString("\\x%1").arg(c0, 2, 16, QChar('0')));
        }
        ui->textEdit->append(str);
        ui->textEdit->verticalScrollBar()->setValue(ui->textEdit->verticalScrollBar()->maximum());
    }
}
