//
// Serge Spraiter 20150608
//

#include <QtWidgets>
#include <QFileInfo>
#include <QFile>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "clientthread.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow), file(0), stopThread(0)
{
    ui->setupUi(this);
    clientThread = 0;
    tcpSocket = new QTcpSocket(this);
    connect(tcpSocket, SIGNAL(disconnected()), this, SLOT(on_disconnected()));
    connect(tcpSocket, SIGNAL(connected()), this, SLOT(on_connected()));
    connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(on_error(QAbstractSocket::SocketError)));
}

MainWindow::~MainWindow()
{
    if (clientThread)
    {
        stopThread = 1;
        if (clientThread->isRunning())
        {
            clientThread->wait(11);
            if (clientThread->isRunning())
                clientThread->terminate();
        }
        delete clientThread;
    }
    if (file)
        delete file;
    delete tcpSocket;
    delete ui;
}

void MainWindow::on_pushButton_Connect_clicked()
{
    if (ui->pushButton_Connect->text() == "Connect")
    {
        if (ui->lineEditIP->text().indexOf(':') < 0)
        {
            QMessageBox::critical(this, "Client1", "Missing ':' in IP:Port");
            return;
        }
        QStringList split = ui->lineEditIP->text().split( ":" );
        bool ok;
        quint32 port = split[1].toUInt(&ok);
        if ((ok == false) || (port > 65535))
        {
            QMessageBox::critical(this, "Client1", "Invalid Port, must be 0..65535");
            return;
        }
        QHostAddress adr;
        if (adr.setAddress(split[0]) == false)
        {
            QMessageBox::critical(this, "Client1", "Invalid IP Address");
            return;
        }
        if (ui->lineEditFile->text() == "")
        {
            QMessageBox::critical(this, "Client1", "File name is <empty>");
            return;
        }
        QFileInfo fileInfo(ui->lineEditFile->text());
        if (!fileInfo.exists() || !fileInfo.isFile())
        {
            QMessageBox::critical(this, "Client1",
                                  QString("File doesn't exist:\n%1").arg(ui->lineEditFile->text()));
            return;
        }
        interval = ui->lineEditInterval->text().toUInt(&ok);
        if ((ok == false) || (interval > 99999))
        {
            QMessageBox::critical(this, "Client1", "Invalid interval, must be 0..99999");
            return;
        }
        fileSize = fileInfo.size();
        ui->textEdit->append(QString("File size=%1").arg(fileSize));
        ui->textEdit->append(QString("Connecting to %1:%2...").arg(adr.toString(), QString("%1").arg(port)));
        ui->textEdit->verticalScrollBar()->setValue(ui->textEdit->verticalScrollBar()->maximum());
        tcpSocket->connectToHost(adr, port);
    }
    else
    {
        tcpSocket->disconnectFromHost();
    }
}

void MainWindow::on_pushButton_Browse_clicked()
{
    QProcess process;
    QString str = QString("ls -al %1").arg(ui->lineEditFile->text());
    ui->textEdit->append(str);
    process.start(str);
    QByteArray buffer;
    while (process.waitForFinished())
        buffer.append(process.readAll());
    ui->textEdit->append(buffer);
    ui->textEdit->verticalScrollBar()->setValue(ui->textEdit->verticalScrollBar()->maximum());
}

void MainWindow::on_pushButton_Send_clicked()
{
    if (ui->pushButton_Send->text() == "Send")
    {
        if (!file)
        {
            file = new QFile(ui->lineEditFile->text());
            if (!file->open(QIODevice::ReadOnly))
            {
                file = 0;
                QMessageBox::critical(this, "Client1",
                                      QString("Unable to open file %1").arg(ui->lineEditFile->text()));
                return;
            }
        }
        ui->textEdit->append(QString("Position=%1 (0x%2)").arg(QString("%1").arg(posInFile),
                                                                QString("%1").arg(posInFile)));
        ui->textEdit->verticalScrollBar()->setValue(ui->textEdit->verticalScrollBar()->maximum());
        // thread
        clientThread = new ClientThread(this, tcpSocket->socketDescriptor(), interval, file,
                                        &posInFile, &stopThread);
        connect(clientThread, SIGNAL(finished()), this, SLOT(on_finished()));
        connect(clientThread, SIGNAL(updateStatusBar(QString)), this, SLOT(on_updateStatusBar(QString)));
        clientThread->start();
        ui->pushButton_Send->setText("Stop");
    }
    else
    {
        stopThread = 1;
    }
}

void MainWindow::on_disconnected()
{
    ui->labelIP->setEnabled(true);
    ui->lineEditIP->setEnabled(true);
    ui->pushButton_Connect->setText("Connect");
    ui->labelFile->setEnabled(true);
    ui->lineEditFile->setEnabled(true);
    ui->pushButton_Browse->setEnabled(true);
    ui->labelInterval->setEnabled(true);
    ui->lineEditInterval->setEnabled(true);
    ui->pushButton_Send->setEnabled(false);
    ui->textEdit->append("Disconnected");
    ui->textEdit->verticalScrollBar()->setValue(ui->textEdit->verticalScrollBar()->maximum());
    file = 0;
}

void MainWindow::on_connected()
{
    ui->labelIP->setEnabled(false);
    ui->lineEditIP->setEnabled(false);
    ui->pushButton_Connect->setText("Disconnect");
    ui->labelFile->setEnabled(false);
    ui->lineEditFile->setEnabled(false);
    ui->pushButton_Browse->setEnabled(false);
    ui->labelInterval->setEnabled(false);
    ui->lineEditInterval->setEnabled(false);
    ui->pushButton_Send->setEnabled(true);
    posInFile = 0;  // from start
    posWas = 0;
    ui->textEdit->append("Connected");
    ui->textEdit->verticalScrollBar()->setValue(ui->textEdit->verticalScrollBar()->maximum());
}

void MainWindow::on_error(QAbstractSocket::SocketError socketError)
{
    switch (socketError) {
    case QAbstractSocket::RemoteHostClosedError:
        break;
    case QAbstractSocket::HostNotFoundError:
        ui->textEdit->append(QString("Host was not found. Check IP:Port"));
        break;
    case QAbstractSocket::ConnectionRefusedError:
        ui->textEdit->append(QString("Host refuses connection. Check that host is running"));
        break;
    default:
        ui->textEdit->append(QString("Error: %1").arg(tcpSocket->errorString()));
    }
    ui->textEdit->verticalScrollBar()->setValue(ui->textEdit->verticalScrollBar()->maximum());
    on_disconnected();
}

void MainWindow::on_finished()
{
    quint64 sent = posInFile - posWas;
    posWas = posInFile;
    ui->textEdit->append(QString("Bytes sent=%1").arg(sent));
    if (posInFile >= fileSize)
    {
        posInFile = 0;
        posWas = 0;
        ui->textEdit->append(QString("End of file, size=%1").arg(fileSize));
    }
    ui->textEdit->verticalScrollBar()->setValue(ui->textEdit->verticalScrollBar()->maximum());
    ui->pushButton_Send->setText("Send");
    stopThread = 0;
}

void MainWindow::on_updateStatusBar(const QString &str)
{
    statusBar()->showMessage(str);
}
